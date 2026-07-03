#include "Framework/Input/InputManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogGFInputManager, Log, All);

void UInputManager::Initialize(AGFPlayerController* InOwner)
{
	OwnerController = InOwner;

	if (OwnerController == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("InputManager initialized without an owner controller. InputManager=%s"), *GetNameSafe(this));
		return;
	}

	CacheLocalPlayerSubsystem();
	ApplyMappingContexts();
}

void UInputManager::SetInputComponent(UEnhancedInputComponent* InInputComponent)
{
	if (EnhancedInputComponent == InInputComponent)
	{
		return;
	}
	
	EnhancedInputComponent = InInputComponent;
	BindConfiguredActions();
}

void UInputManager::ApplyMappingContexts()
{
	if (MappingContexts.IsEmpty())
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Input mapping contexts are not configured. InputManager=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(OwnerController));
		return;
	}

	if (LocalPlayerSubsystem == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot apply mapping contexts because LocalPlayerSubsystem is not initialized. InputManager=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(OwnerController));
		return;
	}

	for (const FInputMappingContextConfig& Config : MappingContexts)
	{
		if (Config.MappingContext == nullptr)
		{
			UE_LOG(LogGFInputManager, Warning, TEXT("Cannot add a null input mapping context. InputManager=%s"), *GetNameSafe(this));
			continue;
		}

		if (ActiveMappingContexts.Contains(Config.MappingContext))
		{
			continue;
		}

		LocalPlayerSubsystem->AddMappingContext(Config.MappingContext, Config.Priority);
		ActiveMappingContexts.Add(Config.MappingContext);
	}
}

void UInputManager::ClearMappingContexts()
{
	if (ActiveMappingContexts.IsEmpty())
	{
		return;
	}

	if (LocalPlayerSubsystem == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot clear mapping contexts because LocalPlayerSubsystem is not initialized. InputManager=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(OwnerController));
		ActiveMappingContexts.Reset();
		return;
	}

	for (UInputMappingContext* MappingContext : ActiveMappingContexts)
	{
		if (MappingContext != nullptr)
		{
			LocalPlayerSubsystem->RemoveMappingContext(MappingContext);
		}
	}

	ActiveMappingContexts.Reset();
}

void UInputManager::BindConfiguredActions()
{
	ClearActionBindings();
	BindingRecords.Reset();

	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot bind input actions without an EnhancedInputComponent. InputManager=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(OwnerController));
		return;
	}

	if (InputActionTable == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Input action table is not configured. InputManager=%s Controller=%s"), *GetNameSafe(this), *GetNameSafe(OwnerController));
		return;
	}

	TArray<FInputActionTableRow*> Rows;
	InputActionTable->GetAllRows(TEXT("InputManager.BindConfiguredActions"), Rows);

	if (Rows.IsEmpty())
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Input action table has no rows. InputManager=%s Table=%s"), *GetNameSafe(this), *GetNameSafe(InputActionTable));
		return;
	}

	for (const FInputActionTableRow* Row : Rows)
	{
		if (Row != nullptr)
		{
			AddConfiguredBindingRecord(*Row);
		}
	}
}

void UInputManager::HandleMoveInput(const FInputActionValue& Value)
{
	OnMoveInput.Broadcast(Value.Get<FVector2D>());
}

void UInputManager::HandleLookInput(const FInputActionValue& Value)
{
	OnLookInput.Broadcast(Value.Get<FVector2D>());
}

void UInputManager::HandleJumpInput(const FInputActionValue&)
{
	OnJumpInput.Broadcast();
}

void UInputManager::HandleAttackInput(const FInputActionValue&)
{
	OnAttackInput.Broadcast();
}

void UInputManager::CacheLocalPlayerSubsystem()
{
	LocalPlayerSubsystem = nullptr;

	if (OwnerController == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot cache input subsystem without an owner controller. InputManager=%s"), *GetNameSafe(this));
		return;
	}

	ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Controller %s has no local player, cannot configure Enhanced Input subsystem."), *GetNameSafe(OwnerController));
		return;
	}

	LocalPlayerSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (LocalPlayerSubsystem == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Enhanced Input local player subsystem is missing. Controller=%s"), *GetNameSafe(OwnerController));
	}
}

void UInputManager::AddConfiguredBindingRecord(const FInputActionTableRow& Row)
{
	if (Row.InputAction == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot bind a null input action. InputManager=%s"), *GetNameSafe(this));
		return;
	}

	FInputActionBindingRecord Record;
	Record.ActionType = Row.ActionType;
	Record.InputAction = Row.InputAction;
	Record.TriggerEvent = Row.TriggerEvent;

	for (FInputActionBindingRecord& ExistingRecord : BindingRecords)
	{
		if (ExistingRecord.ActionType == Record.ActionType &&
			ExistingRecord.InputAction == Record.InputAction &&
			ExistingRecord.TriggerEvent == Record.TriggerEvent)
		{
			Warning
			UnbindRecord(ExistingRecord);
			ExistingRecord = Record;
			BindRecord(ExistingRecord);
			return;
		}
	}

	FInputActionBindingRecord& NewRecord = BindingRecords.Add_GetRef(Record);
	BindRecord(NewRecord);
}

void UInputManager::BindRecord(FInputActionBindingRecord& Record)
{
	if (Record.bIsBound || !CanBindRecord(Record))
	{
		return;
	}

	const int32 RecordIndex = BindingRecords.IndexOfByPredicate([&Record](const FInputActionBindingRecord& Candidate)
	{
		return &Candidate == &Record;
	});

	if (RecordIndex == INDEX_NONE)
	{
		return;
	}

	FEnhancedInputActionEventBinding* Binding = nullptr;

	// 根据 DataTable 中的动作类型绑定到固定 C++ 处理函数，避免外部参与 Enhanced Input 绑定细节。
	switch (Record.ActionType)
	{
	case EInputActionType::Move:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UInputManager::HandleMoveInput);
		break;
	case EInputActionType::Look:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UInputManager::HandleLookInput);
		break;
	case EInputActionType::Jump:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UInputManager::HandleJumpInput);
		break;
	case EInputActionType::Attack:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UInputManager::HandleAttackInput);
		break;
	case EInputActionType::None:
	default:
		UE_LOG(LogGFInputManager, Warning, TEXT("Unsupported input action type. InputManager=%s Action=%s"), *GetNameSafe(this), *GetNameSafe(Record.InputAction));
		break;
	}

	if (Binding == nullptr)
	{
		return;
	}

	Record.BindingHandle = Binding->GetHandle();
	Record.bIsBound = true;
}

void UInputManager::UnbindRecord(FInputActionBindingRecord& Record)
{
	if (EnhancedInputComponent == nullptr || !Record.bIsBound)
	{
		Record.BindingHandle = 0;
		Record.bIsBound = false;
		return;
	}

	EnhancedInputComponent->RemoveBindingByHandle(Record.BindingHandle);
	Record.BindingHandle = 0;
	Record.bIsBound = false;
}

void UInputManager::ClearActionBindings()
{
	for (FInputActionBindingRecord& Record : BindingRecords)
	{
		UnbindRecord(Record);
	}
}

bool UInputManager::CanBindRecord(const FInputActionBindingRecord& Record) const
{
	return EnhancedInputComponent != nullptr && Record.InputAction != nullptr;
}

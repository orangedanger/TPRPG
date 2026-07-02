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
	RefreshLocalPlayerSubsystem();
	ApplyMappingContexts();
	BindConfiguredActions();
}

void UInputManager::SetInputComponent(UEnhancedInputComponent* InInputComponent)
{
	if (EnhancedInputComponent == InInputComponent)
	{
		return;
	}

	ClearBindings();
	EnhancedInputComponent = InInputComponent;
	BindConfiguredActions();
}

void UInputManager::RefreshLocalPlayerSubsystem()
{
	LocalPlayerSubsystem = nullptr;

	if (OwnerController == nullptr)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Cannot refresh input subsystem without an owner controller."));
		return;
	}

	ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Controller %s has no local player."), *GetNameSafe(OwnerController));
		return;
	}

	LocalPlayerSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

void UInputManager::ApplyMappingContexts()
{
	if (LocalPlayerSubsystem == nullptr)
	{
		RefreshLocalPlayerSubsystem();
	}

	if (LocalPlayerSubsystem == nullptr)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Cannot apply mapping contexts without a local player subsystem."));
		return;
	}

	for (const FInputMappingContextConfig& Config : MappingContexts)
	{
		if (Config.MappingContext == nullptr)
		{
			UE_LOG(LogGFInputManager, Warning, TEXT("Cannot add a null input mapping context."));
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
	if (LocalPlayerSubsystem == nullptr)
	{
		RefreshLocalPlayerSubsystem();
	}

	if (LocalPlayerSubsystem)
	{
		for (UInputMappingContext* MappingContext : ActiveMappingContexts)
		{
			if (MappingContext != nullptr)
			{
				LocalPlayerSubsystem->RemoveMappingContext(MappingContext);
			}
		}
	}

	ActiveMappingContexts.Reset();
}

void UInputManager::BindConfiguredActions()
{
	ClearBindings();
	BindingRecords.Reset();

	if (InputActionTable == nullptr)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Input action table is not configured."));
		return;
	}

	TArray<FInputActionTableRow*> Rows;
	InputActionTable->GetAllRows(TEXT("InputManager.BindConfiguredActions"), Rows);

	for (const FInputActionTableRow* Row : Rows)
	{
		if (Row != nullptr)
		{
			AddConfiguredBindingRecord(*Row);
		}
	}
}

void UInputManager::DisableInputGroup(EInputGroup InputGroup)
{
	if (InputGroup == EInputGroup::None)
	{
		return;
	}

	DisabledGroups.Add(InputGroup);

	for (FInputActionBindingRecord& Record : BindingRecords)
	{
		if (Record.InputGroup == InputGroup)
		{
			UnbindRecord(Record);
		}
	}
}

void UInputManager::EnableInputGroup(EInputGroup InputGroup)
{
	if (InputGroup == EInputGroup::None)
	{
		return;
	}

	DisabledGroups.Remove(InputGroup);

	for (FInputActionBindingRecord& Record : BindingRecords)
	{
		if (Record.InputGroup == InputGroup)
		{
			BindRecord(Record);
		}
	}
}

void UInputManager::ClearBindings()
{
	for (FInputActionBindingRecord& Record : BindingRecords)
	{
		UnbindRecord(Record);
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

void UInputManager::AddConfiguredBindingRecord(const FInputActionTableRow& Row)
{
	if (Row.InputAction == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot bind a null input action."));
		return;
	}

	FInputActionBindingRecord Record;
	Record.ActionType = Row.ActionType;
	Record.InputAction = Row.InputAction;
	Record.TriggerEvent = Row.TriggerEvent;
	Record.InputGroup = Row.InputGroup;

	for (FInputActionBindingRecord& ExistingRecord : BindingRecords)
	{
		if (ExistingRecord.ActionType == Record.ActionType &&
			ExistingRecord.InputAction == Record.InputAction &&
			ExistingRecord.TriggerEvent == Record.TriggerEvent &&
			ExistingRecord.InputGroup == Record.InputGroup)
		{
			UnbindRecord(ExistingRecord);
			ExistingRecord = Record;

			if (!DisabledGroups.Contains(ExistingRecord.InputGroup))
			{
				BindRecord(ExistingRecord);
			}
			return;
		}
	}

	FInputActionBindingRecord& NewRecord = BindingRecords.Add_GetRef(Record);
	if (!DisabledGroups.Contains(NewRecord.InputGroup))
	{
		BindRecord(NewRecord);
	}
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

bool UInputManager::CanBindRecord(const FInputActionBindingRecord& Record) const
{
	return EnhancedInputComponent != nullptr && Record.InputAction != nullptr;
}

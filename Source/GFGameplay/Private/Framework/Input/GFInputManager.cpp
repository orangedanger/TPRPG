#include "Framework/Input/GFInputManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Input/GFInputDelegates.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogGFInputManager, Log, All);

void UGFInputManager::Initialize(AGFPlayerController* InOwner)
{
	OwnerController = InOwner;

	if (OwnerController == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("GFInputManager initialized without an owner controller. InputManager=%s"), *GetNameSafe(this));
		return;
	}

	CacheLocalPlayerSubsystem();
	ApplyMappingContexts();
}

void UGFInputManager::SetInputComponent(UEnhancedInputComponent* InInputComponent)
{
	if (EnhancedInputComponent == InInputComponent)
	{
		return;
	}

	ClearActionBindings();
	EnhancedInputComponent = InInputComponent;
	BindConfiguredActions();
}

void UGFInputManager::UpdateInputActions(float DeltaTime)
{
	if (PendingInputTags.IsEmpty())
	{
		return;
	}

	TArray<EGFInputActionTag> TagsToProcess;
	Swap(TagsToProcess, PendingInputTags);

	for (const EGFInputActionTag InputTag : TagsToProcess)
	{
		ProcessInputAction(InputTag);
	}
}

void UGFInputManager::ApplyMappingContexts()
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

void UGFInputManager::ClearMappingContexts()
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

void UGFInputManager::BindConfiguredActions()
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
	InputActionTable->GetAllRows(TEXT("GFInputManager.BindConfiguredActions"), Rows);

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

void UGFInputManager::HandleMoveInput(const FInputActionValue& Value)
{
	if (OwnerController == nullptr || OwnerController->GetInputDelegates() == nullptr)
	{
		return;
	}

	OwnerController->GetInputDelegates()->OnMoveInput.Broadcast(Value.Get<FVector2D>());
}

void UGFInputManager::HandleLookInput(const FInputActionValue& Value)
{
	if (OwnerController == nullptr || OwnerController->GetInputDelegates() == nullptr)
	{
		return;
	}

	OwnerController->GetInputDelegates()->OnLookInput.Broadcast(Value.Get<FVector2D>());
}

void UGFInputManager::HandleJumpPressedInput()
{
	QueueInputAction(EGFInputActionTag::Input_Jump_Pressed);
}

void UGFInputManager::HandleJumpReleasedInput()
{
	QueueInputAction(EGFInputActionTag::Input_Jump_Released);
}

void UGFInputManager::HandleAttackPressedInput()
{
	QueueInputAction(EGFInputActionTag::Input_Attack_Pressed);
}

void UGFInputManager::HandleAttackReleasedInput()
{
	QueueInputAction(EGFInputActionTag::Input_Attack_Released);
}

void UGFInputManager::CacheLocalPlayerSubsystem()
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

	LocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (LocalPlayerSubsystem == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Enhanced Input local player subsystem is missing. Controller=%s"), *GetNameSafe(OwnerController));
	}
}

void UGFInputManager::AddConfiguredBindingRecord(const FInputActionTableRow& Row)
{
	if (Row.InputAction == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Cannot bind a null input action. InputManager=%s"), *GetNameSafe(this));
		return;
	}

	switch (Row.ActionType)
	{
	case EInputActionType::Move:
	case EInputActionType::Look:
		AddBindingRecord(Row.ActionType, Row.InputAction, Row.TriggerEvent);
		break;
	case EInputActionType::Jump:
		AddBindingRecord(Row.ActionType, Row.InputAction, ETriggerEvent::Started);
		AddBindingRecord(Row.ActionType, Row.InputAction, ETriggerEvent::Completed);
		break;
	case EInputActionType::Attack:
		AddBindingRecord(Row.ActionType, Row.InputAction, ETriggerEvent::Started);
		AddBindingRecord(Row.ActionType, Row.InputAction, ETriggerEvent::Completed);
		break;
	case EInputActionType::None:
	default:
		UE_LOG(LogGFInputManager, Warning, TEXT("Unsupported input action type. InputManager=%s Action=%s"), *GetNameSafe(this), *GetNameSafe(Row.InputAction));
		break;
	}
}

void UGFInputManager::AddBindingRecord(EInputActionType ActionType, UInputAction* InputAction, ETriggerEvent TriggerEvent)
{
	FInputActionBindingRecord Record;
	Record.ActionType = ActionType;
	Record.InputAction = InputAction;
	Record.TriggerEvent = TriggerEvent;

	for (FInputActionBindingRecord& ExistingRecord : BindingRecords)
	{
		if (ExistingRecord.ActionType == Record.ActionType &&
			ExistingRecord.InputAction == Record.InputAction &&
			ExistingRecord.TriggerEvent == Record.TriggerEvent)
		{
			// TODO: 当前重复配置会以后者覆盖前者；后续需要明确是支持多绑定、拒绝重复配置，还是保留覆盖策略。
			UnbindRecord(ExistingRecord);
			ExistingRecord = Record;
			BindRecord(ExistingRecord);
			return;
		}
	}

	FInputActionBindingRecord& NewRecord = BindingRecords.Add_GetRef(Record);
	BindRecord(NewRecord);
}

void UGFInputManager::BindRecord(FInputActionBindingRecord& Record)
{
	if (Record.bIsBound || CanBindRecord(Record) == false)
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

	switch (Record.ActionType)
	{
	case EInputActionType::Move:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleMoveInput);
		break;
	case EInputActionType::Look:
		Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleLookInput);
		break;
	case EInputActionType::Jump:
		if (Record.TriggerEvent == ETriggerEvent::Started)
		{
			Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleJumpPressedInput);
		}
		else if (Record.TriggerEvent == ETriggerEvent::Completed)
		{
			Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleJumpReleasedInput);
		}
		break;
	case EInputActionType::Attack:
		if (Record.TriggerEvent == ETriggerEvent::Started)
		{
			Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleAttackPressedInput);
		}
		else if (Record.TriggerEvent == ETriggerEvent::Completed)
		{
			Binding = &EnhancedInputComponent->BindAction(Record.InputAction, Record.TriggerEvent, this, &UGFInputManager::HandleAttackReleasedInput);
		}
		break;
	case EInputActionType::None:
	default:
		break;
	}

	if (Binding == nullptr)
	{
		UE_LOG(LogGFInputManager, Warning, TEXT("Failed to bind input record. InputManager=%s Action=%s Type=%d Event=%d"), *GetNameSafe(this), *GetNameSafe(Record.InputAction), static_cast<int32>(Record.ActionType), static_cast<int32>(Record.TriggerEvent));
		return;
	}

	Record.BindingHandle = Binding->GetHandle();
	Record.bIsBound = true;
}

void UGFInputManager::UnbindRecord(FInputActionBindingRecord& Record)
{
	if (EnhancedInputComponent == nullptr || Record.bIsBound == false)
	{
		Record.BindingHandle = 0;
		Record.bIsBound = false;
		return;
	}

	EnhancedInputComponent->RemoveBindingByHandle(Record.BindingHandle);
	Record.BindingHandle = 0;
	Record.bIsBound = false;
}

void UGFInputManager::ClearActionBindings()
{
	for (FInputActionBindingRecord& Record : BindingRecords)
	{
		UnbindRecord(Record);
	}
}

void UGFInputManager::QueueInputAction(EGFInputActionTag InputTag)
{
	if (InputTag == EGFInputActionTag::None)
	{
		return;
	}

	PendingInputTags.Add(InputTag);
}

void UGFInputManager::ProcessInputAction(EGFInputActionTag InputTag)
{
	if (OwnerController == nullptr || OwnerController->GetInputDelegates() == nullptr)
	{
		return;
	}

	FGFInputDelegates* InputDelegates = OwnerController->GetInputDelegates();
	switch (InputTag)
	{
	case EGFInputActionTag::Input_Jump_Pressed:
		InputDelegates->OnJumpPressed.Broadcast();
		break;
	case EGFInputActionTag::Input_Jump_Released:
		InputDelegates->OnJumpReleased.Broadcast();
		break;
	case EGFInputActionTag::Input_Attack_Pressed:
		InputDelegates->OnAttackPressed.Broadcast();
		break;
	case EGFInputActionTag::Input_Attack_Released:
		InputDelegates->OnAttackReleased.Broadcast();
		break;
	case EGFInputActionTag::None:
	default:
		break;
	}
}

bool UGFInputManager::CanBindRecord(const FInputActionBindingRecord& Record) const
{
	return EnhancedInputComponent != nullptr && Record.InputAction != nullptr;
}

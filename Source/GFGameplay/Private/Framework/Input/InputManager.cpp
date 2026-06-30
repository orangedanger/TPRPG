#include "Framework/Input/InputManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "GameFramework/Character.h"
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

	if (!OwnerController)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Cannot refresh input subsystem without an owner controller."));
		return;
	}

	ULocalPlayer* LocalPlayer = OwnerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Controller %s has no local player."), *GetNameSafe(OwnerController));
		return;
	}

	LocalPlayerSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

void UInputManager::ApplyMappingContexts()
{
	if (!LocalPlayerSubsystem)
	{
		RefreshLocalPlayerSubsystem();
	}

	if (!LocalPlayerSubsystem)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Cannot apply mapping contexts without a local player subsystem."));
		return;
	}

	for (const FInputMappingContextConfig& Config : MappingContexts)
	{
		if (!Config.MappingContext)
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
	if (!LocalPlayerSubsystem)
	{
		RefreshLocalPlayerSubsystem();
	}

	if (LocalPlayerSubsystem)
	{
		for (UInputMappingContext* MappingContext : ActiveMappingContexts)
		{
			if (MappingContext)
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

	if (!InputActionTable)
	{
		UE_LOG(LogGFInputManager, Verbose, TEXT("Input action table is not configured."));
		return;
	}

	TArray<FInputActionTableRow*> Rows;
	InputActionTable->GetAllRows(TEXT("InputManager.BindConfiguredActions"), Rows);

	for (const FInputActionTableRow* Row : Rows)
	{
		if (Row)
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
	if (!OwnerController)
	{
		return;
	}

	APawn* ControlledPawn = OwnerController->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = OwnerController->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// TODO: 后续通过委托广播输入事件，让移动逻辑在角色或组件中完成。
	ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
}

void UInputManager::HandleLookInput(const FInputActionValue& Value)
{
	if (!OwnerController)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// TODO: 后续通过委托广播输入事件，让镜头逻辑在角色或相机组件中完成。
	OwnerController->AddYawInput(LookAxisVector.X);
	OwnerController->AddPitchInput(LookAxisVector.Y);
}

void UInputManager::HandleJumpInput(const FInputActionValue& Value)
{
	if (!OwnerController)
	{
		return;
	}

	ACharacter* ControlledCharacter = Cast<ACharacter>(OwnerController->GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	// TODO: 后续通过委托广播输入事件，让跳跃逻辑在角色或移动组件中完成。
	ControlledCharacter->Jump();
}

void UInputManager::HandleAttackInput(const FInputActionValue& Value)
{
	// TODO: 后续通过委托广播输入事件，让攻击逻辑在战斗组件或能力系统中完成。
	UE_LOG(LogGFInputManager, Log, TEXT("Attack input triggered."));
}

void UInputManager::AddConfiguredBindingRecord(const FInputActionTableRow& Row)
{
	if (!Row.InputAction)
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

	// 根据 DataTable 中的动作类型绑定到固定的 C++ 处理函数，避免外部参与绑定。
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

	if (!Binding)
	{
		return;
	}

	Record.BindingHandle = Binding->GetHandle();
	Record.bIsBound = true;
}

void UInputManager::UnbindRecord(FInputActionBindingRecord& Record)
{
	if (!EnhancedInputComponent || !Record.bIsBound)
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
	return EnhancedInputComponent && Record.InputAction;
}

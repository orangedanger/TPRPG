#include "Framework/PlayerController/GFPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Framework/Input/InputManager.h"

AGFPlayerController::AGFPlayerController()
{
	bReplicates = true;
}

void AGFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializeInputManager();
	if (InputManager != nullptr)
	{
		InputManager->RefreshLocalPlayerSubsystem();
	}
}

void AGFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	InitializeInputManager();
	if (InputManager != nullptr)
	{
		InputManager->RefreshLocalPlayerSubsystem();
	}
}

void AGFPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	InitializeInputManager();
	if (InputManager != nullptr)
	{
		InputManager->RefreshLocalPlayerSubsystem();
	}
}

void AGFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitializeInputManager();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (InputManager != nullptr && EnhancedInputComponent != nullptr)
	{
		InputManager->SetInputComponent(EnhancedInputComponent);
	}
}

void AGFPlayerController::InitializeInputManager()
{
	if (InputManager == nullptr)
	{
		// Allow Blueprint controllers to provide a configured InputManager subclass.
		TSubclassOf<UInputManager> ClassToCreate = InputManagerClass;
		if (ClassToCreate == nullptr)
		{
			ClassToCreate = UInputManager::StaticClass();
		}
		InputManager = NewObject<UInputManager>(this, ClassToCreate, TEXT("InputManager"));
	}

	if (InputManager != nullptr)
	{
		InputManager->Initialize(this);
	}
}

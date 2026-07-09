#include "Framework/PlayerController/GFPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Framework/Input/GFInputManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGFPlayerController, Log, All);

AGFPlayerController::AGFPlayerController()
{
	bReplicates = true;
	InputManagerClass = UGFInputManager::StaticClass();
}

void AGFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializeInput();
}

void AGFPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputManager != nullptr)
	{
		InputManager->Deinitialize();
		InputManager = nullptr;
	}

	ConfiguredEnhancedInputComponent = nullptr;
	bInputInitialized = false;

	Super::EndPlay(EndPlayReason);
}

void AGFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitializeInput();

	if (IsLocalController() == false)
	{
		return;
	}

	if (InputManager == nullptr)
	{
		UE_LOG(LogGFPlayerController, Verbose, TEXT("InputManager is not initialized yet. Controller=%s"), *GetNameSafe(this));
		return;
	}

	if (InputComponent == nullptr)
	{
		UE_LOG(LogGFPlayerController, Verbose, TEXT("InputComponent is not created yet. Controller=%s"), *GetNameSafe(this));
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogGFPlayerController, Warning, TEXT("InputComponent is not UEnhancedInputComponent. Check DefaultInput.ini. Controller=%s InputComponent=%s"), *GetNameSafe(this), *GetNameSafe(InputComponent));
		return;
	}

	if (ConfiguredEnhancedInputComponent == EnhancedInputComponent)
	{
		return;
	}

	InputManager->SetInputComponent(EnhancedInputComponent);
	ConfiguredEnhancedInputComponent = EnhancedInputComponent;
}

void AGFPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController() == false || InputManager == nullptr)
	{
		return;
	}

	InputManager->UpdateInputActions(DeltaTime);
}

void AGFPlayerController::InitializeInput()
{
	if (bInputInitialized)
	{
		return;
	}

	if (IsLocalController() == false)
	{
		return;
	}

	if (InputManagerClass == nullptr)
	{
		UE_LOG(LogGFPlayerController, Warning, TEXT("InputManagerClass is not configured. Controller=%s"), *GetNameSafe(this));
	}
	else if (InputManagerClass->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogGFPlayerController, Warning, TEXT("InputManagerClass is abstract and cannot be created. Controller=%s Class=%s"), *GetNameSafe(this), *GetNameSafe(InputManagerClass));
	}
	else
	{
		InputManager = NewObject<UGFInputManager>(this, InputManagerClass, TEXT("InputManager"));
		if (InputManager == nullptr)
		{
			UE_LOG(LogGFPlayerController, Warning, TEXT("Failed to create input manager. Controller=%s Class=%s"), *GetNameSafe(this), *GetNameSafe(InputManagerClass));
		}
		else
		{
			InputManager->Initialize(this);
		}
	}

	bInputInitialized = true;
}

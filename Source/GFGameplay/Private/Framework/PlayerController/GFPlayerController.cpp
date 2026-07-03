#include "Framework/PlayerController/GFPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Framework/Input/InputManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGFPlayerController, Log, All);

AGFPlayerController::AGFPlayerController()
{
	bReplicates = true;
}

void AGFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializeInputManager();
}

void AGFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController())
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

	InputManager->SetInputComponent(EnhancedInputComponent);
}

void AGFPlayerController::InitializeInputManager()
{
	if (!IsLocalController())
	{
		return;
	}

	if (InputManager != nullptr)
	{
		return;
	}
	// 蓝图控制器通常在这里指定带输入资源配置的 UInputManager 子类。
	TSubclassOf<UInputManager> ClassToCreate = InputManagerClass;
	if (ClassToCreate == nullptr)
	{
		UE_LOG(LogGFPlayerController, Warning, TEXT("InputManagerClass is not configured on %s. Falling back to UInputManager without Blueprint input assets."), *GetNameSafe(this));
		ClassToCreate = UInputManager::StaticClass();
	}

	InputManager = NewObject<UInputManager>(this, ClassToCreate, TEXT("InputManager"));
	if (InputManager == nullptr)
	{
		UE_LOG(LogGFPlayerController, Warning, TEXT("Failed to create InputManager. Controller=%s Class=%s"), *GetNameSafe(this), *GetNameSafe(ClassToCreate));
		return;
	}

	InputManager->Initialize(this);
}

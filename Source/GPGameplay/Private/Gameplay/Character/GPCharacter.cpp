#include "Gameplay/Character/GPCharacter.h"

#include "Framework/Input/InputManager.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "Gameplay/Components/GPAttributeSetComponent.h"
#include "Gameplay/Components/GPCombatComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPCharacter, Log, All);

AGPCharacter::AGPCharacter()
{
	AttributeSetComponent = CreateDefaultSubobject<UGPAttributeSetComponent>(TEXT("AttributeSetComponent"));
	CombatComponent = CreateDefaultSubobject<UGPCombatComponent>(TEXT("CombatComponent"));
}

// Begin AActor interface
void AGPCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshInputBinding();
}

void AGPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearInputBinding();

	Super::EndPlay(EndPlayReason);
}
// End AActor interface

// Begin APawn interface
void AGPCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	RefreshInputBinding();
}
// End APawn interface

/**
 * IDamageManagerInterface Begin
 */
void AGPCharacter::MakeDamage(AActor* TargetActor, float DamageAmount, const FHitResult& HitResult)
{
	if (TargetActor == nullptr || TargetActor == this)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("MakeDamage skipped. Instigator=%s Target=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(TargetActor), DamageAmount);
		return;
	}

	IDamageManagerInterface* DamageReceiver = Cast<IDamageManagerInterface>(TargetActor);
	if (DamageReceiver == nullptr)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("MakeDamage target does not implement IDamageManagerInterface. Instigator=%s Target=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(TargetActor), DamageAmount);
		return;
	}

	UE_LOG(LogGPCharacter, Log, TEXT("MakeDamage. Instigator=%s Target=%s Damage=%.2f Impact=%s"), *GetNameSafe(this), *GetNameSafe(TargetActor), DamageAmount, *HitResult.ImpactPoint.ToString());
	DamageReceiver->TakeDamage(this, this, DamageAmount, HitResult);
}

void AGPCharacter::TakeDamage(AActor* DamageInstigator, AActor* DamageCauser, float DamageAmount, const FHitResult& HitResult)
{
	if (AttributeSetComponent == nullptr)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped because AttributeSetComponent is null. Receiver=%s Instigator=%s Causer=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount);
		return;
	}

	const float OldHealth = AttributeSetComponent->GetHealth();
	UE_LOG(LogGPCharacter, Log, TEXT("TakeDamage. Receiver=%s Instigator=%s Causer=%s Damage=%.2f OldHealth=%.2f Impact=%s"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount, OldHealth, *HitResult.ImpactPoint.ToString());
	AttributeSetComponent->SetHealth(OldHealth - DamageAmount);
}
/**
 * IDamageManagerInterface End
 */

// Begin input delegate handlers
void AGPCharacter::RefreshInputBinding()
{
	ClearInputBinding();

	AGFPlayerController* PlayerController = Cast<AGFPlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	UInputManager* InputManager = PlayerController->GetInputManager();
	if (InputManager == nullptr)
	{
		return;
	}

	BoundInputManager = InputManager;
	BoundInputManager->OnMoveInput.AddUObject(this, &AGPCharacter::HandleMoveInput);
	BoundInputManager->OnLookInput.AddUObject(this, &AGPCharacter::HandleLookInput);
	BoundInputManager->OnJumpInput.AddUObject(this, &AGPCharacter::HandleJumpInput);
	BoundInputManager->OnAttackInput.AddUObject(this, &AGPCharacter::HandleAttackInput);
}

void AGPCharacter::ClearInputBinding()
{
	if (BoundInputManager == nullptr)
	{
		return;
	}

	BoundInputManager->OnMoveInput.RemoveAll(this);
	BoundInputManager->OnLookInput.RemoveAll(this);
	BoundInputManager->OnJumpInput.RemoveAll(this);
	BoundInputManager->OnAttackInput.RemoveAll(this);
	BoundInputManager = nullptr;
}

void AGPCharacter::HandleMoveInput(const FVector2D& MovementVector)
{
	AController* OwnerController = GetController();
	if (OwnerController == nullptr)
	{
		return;
	}

	const FRotator ControlRotation = OwnerController->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AGPCharacter::HandleLookInput(const FVector2D& LookAxisVector)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->AddYawInput(LookAxisVector.X);
	PlayerController->AddPitchInput(LookAxisVector.Y);
}

void AGPCharacter::HandleJumpInput()
{
	Jump();
}

void AGPCharacter::HandleAttackInput()
{
	if (CombatComponent != nullptr)
	{
		UE_LOG(LogGPCharacter, Log, TEXT("Attack input received. Character=%s CombatComponent=%s"), *GetNameSafe(this), *GetNameSafe(CombatComponent));
		CombatComponent->HandleAttackInput();
	}
	else
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("Attack input ignored because CombatComponent is null. Character=%s"), *GetNameSafe(this));
	}
}
// End input delegate handlers

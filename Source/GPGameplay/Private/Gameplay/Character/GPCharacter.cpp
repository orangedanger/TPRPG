#include "Gameplay/Character/GPCharacter.h"

#include "Framework/Input/GFInputDelegates.h"
#include "Gameplay/Components/GPAttributeSetComponent.h"
#include "Gameplay/Components/GPCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/PlayerController/GPPlayerController.h"

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
	
	InitializeAttributeDelegateBindings();
}

void AGPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAttributeDelegateBindings();
	ClearInputDelegateBindings();

	Super::EndPlay(EndPlayReason);
}

void AGPCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	BindInputDelegateBindings();
}

void AGPCharacter::UnPossessed()
{
	ClearInputDelegateBindings();

	Super::UnPossessed();
}

void AGPCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	BindInputDelegateBindings();
}
// End AActor interface

/**
 * IDamageManagerInterface Begin
 */
void AGPCharacter::MakeDamage(AActor* TargetActor, float DamageAmount, const FHitResult& HitResult)
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("MakeDamage skipped on non-authority. Instigator=%s Target=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(TargetActor), DamageAmount);
		return;
	}

	if (IsCharacterDead())
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("MakeDamage skipped because instigator is dead. Instigator=%s Target=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(TargetActor), DamageAmount);
		return;
	}

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
	if (HasAuthority() == false)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped on non-authority. Receiver=%s Instigator=%s Causer=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount);
		return;
	}

	if (AttributeSetComponent == nullptr)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped because AttributeSetComponent is null. Receiver=%s Instigator=%s Causer=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount);
		return;
	}

	if (AttributeSetComponent->IsDead())
	{
		UE_LOG(LogGPCharacter, Log, TEXT("TakeDamage skipped because receiver is already dead. Receiver=%s Instigator=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), DamageAmount);
		return;
	}

	if (DamageAmount <= 0.0f)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped because damage is invalid. Receiver=%s Instigator=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), DamageAmount);
		return;
	}

	const float OldHealth = AttributeSetComponent->GetHealth();
	UE_LOG(LogGPCharacter, Log, TEXT("TakeDamage. Receiver=%s Instigator=%s Causer=%s Damage=%.2f OldHealth=%.2f Impact=%s"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount, OldHealth, *HitResult.ImpactPoint.ToString());
	AttributeSetComponent->SetHealth(OldHealth - DamageAmount);
}
/**
 * IDamageManagerInterface End
 */

// Begin attribute delegate handlers
void AGPCharacter::InitializeAttributeDelegateBindings()
{
	if (AttributeSetComponent == nullptr)
	{
		return;
	}

	AttributeSetComponent->OnOwnerDead.AddDynamic(this, &AGPCharacter::HandleAttributeOwnerDead);
}

void AGPCharacter::ClearAttributeDelegateBindings()
{
	if (AttributeSetComponent == nullptr)
	{
		return;
	}

	AttributeSetComponent->OnOwnerDead.RemoveAll(this);
}

void AGPCharacter::HandleAttributeOwnerDead(AActor* DeadActor)
{
	if (DeadActor != this)
	{
		return;
	}

	ApplyDeathRagdoll();
}

// TODO: 临时死亡效果
void AGPCharacter::ApplyDeathRagdoll()
{
	ClearInputDelegateBindings();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent != nullptr)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	UCapsuleComponent* OwnerCapsule = GetCapsuleComponent();
	if (OwnerCapsule != nullptr)
	{
		OwnerCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OwnerCapsule->SetGenerateOverlapEvents(false);
	}

	USkeletalMeshComponent* OwnerMesh = GetMesh();
	if (OwnerMesh != nullptr)
	{
		OwnerMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		OwnerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		OwnerMesh->SetSimulatePhysics(true);
		OwnerMesh->WakeAllRigidBodies();
	}
}
// End attribute delegate handlers

// Begin input delegate handlers
void AGPCharacter::BindInputDelegateBindings()
{
	if (IsCharacterDead())
	{
		ClearInputDelegateBindings();
		return;
	}
	
	AGPPlayerController* CurrentPlayerController = Cast<AGPPlayerController>(Controller);
	if (CurrentPlayerController == nullptr)
	{
		ClearInputDelegateBindings();
		return;
	}

	if (CurrentPlayerController->IsLocalController() == false)
	{
		ClearInputDelegateBindings();
		return;
	}

	if (PlayerController.Get() == CurrentPlayerController)
	{
		return;
	}

	ClearInputDelegateBindings();

	FGFInputDelegates* InputDelegates = CurrentPlayerController->GetInputDelegates();
	if (InputDelegates == nullptr)
	{
		return;
	}
	
	InputDelegates->OnMoveInput.AddUObject(this, &AGPCharacter::HandleMoveInput);
	InputDelegates->OnLookInput.AddUObject(this, &AGPCharacter::HandleLookInput);
	InputDelegates->OnJumpPressed.AddUObject(this, &AGPCharacter::HandleJumpPressedInput);
	InputDelegates->OnJumpReleased.AddUObject(this, &AGPCharacter::HandleJumpReleasedInput);
	InputDelegates->OnAttackPressed.AddUObject(this, &AGPCharacter::HandleAttackPressedInput);
	PlayerController = CurrentPlayerController;
}

void AGPCharacter::ClearInputDelegateBindings()
{
	AGPPlayerController* BoundPlayerController = PlayerController.Get();
	if (BoundPlayerController == nullptr)
	{
		PlayerController.Reset();
		return;
	}

	FGFInputDelegates* InputDelegates = BoundPlayerController->GetInputDelegates();
	if (InputDelegates != nullptr)
	{
		InputDelegates->OnMoveInput.RemoveAll(this);
		InputDelegates->OnLookInput.RemoveAll(this);
		InputDelegates->OnJumpPressed.RemoveAll(this);
		InputDelegates->OnJumpReleased.RemoveAll(this);
		InputDelegates->OnAttackPressed.RemoveAll(this);
		InputDelegates->OnAttackReleased.RemoveAll(this);
	}

	PlayerController.Reset();
}

bool AGPCharacter::IsCharacterDead() const
{
	return AttributeSetComponent != nullptr && AttributeSetComponent->IsDead();
}

void AGPCharacter::HandleMoveInput(const FVector2D& MovementVector)
{
	if (PlayerController == nullptr || IsCharacterDead())
	{
		return;
	}

	const FRotator ControlRotation = PlayerController->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AGPCharacter::HandleLookInput(const FVector2D& LookAxisVector)
{
	if (PlayerController == nullptr || IsCharacterDead())
	{
		return;
	}

	PlayerController->AddYawInput(LookAxisVector.X);
	PlayerController->AddPitchInput(LookAxisVector.Y);
}

void AGPCharacter::HandleJumpPressedInput()
{
	if (IsCharacterDead())
	{
		return;
	}

	Jump();
}

void AGPCharacter::HandleJumpReleasedInput()
{
	if (IsCharacterDead())
	{
		return;
	}

	StopJumping();
}

void AGPCharacter::HandleAttackPressedInput()
{
	if (IsCharacterDead() || CombatComponent == nullptr)
	{
		return;
	}

	CombatComponent->HandleAttackInput();
}

// End input delegate handlers

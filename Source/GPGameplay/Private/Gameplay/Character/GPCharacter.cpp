#include "Gameplay/Character/GPCharacter.h"

#include "Framework/Input/GFInputDelegates.h"
#include "Gameplay/PlayerController/GPPlayerController.h"
#include "Gameplay/Components/GPAttributeSetComponent.h"
#include "Gameplay/Components/GPCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

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
	InitializeInputDelegateBindings();
}

void AGPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAttributeDelegateBindings();
	ClearInputDelegateBindings();

	Super::EndPlay(EndPlayReason);
}
// End AActor interface

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
void AGPCharacter::InitializeInputDelegateBindings()
{
	ClearInputDelegateBindings();
	
	if (PlayerController == nullptr)
	{
		return;
	}

	FGFInputDelegates* InputDelegates = PlayerController->GetInputDelegates();
	if (InputDelegates == nullptr)
	{
		return;
	}
	
	InputDelegates->OnMoveInput.AddUObject(this, &AGPCharacter::HandleMoveInput);
	InputDelegates->OnLookInput.AddUObject(this, &AGPCharacter::HandleLookInput);
	InputDelegates->OnJumpPressed.AddUObject(this, &AGPCharacter::HandleJumpPressedInput);
	InputDelegates->OnJumpReleased.AddUObject(this, &AGPCharacter::HandleJumpReleasedInput);
}

void AGPCharacter::ClearInputDelegateBindings()
{
	if (PlayerController == nullptr)
	{
		return;
	}

	FGFInputDelegates* InputDelegates = PlayerController->GetInputDelegates();
	if (InputDelegates != nullptr)
	{
		InputDelegates->OnMoveInput.RemoveAll(this);
		InputDelegates->OnLookInput.RemoveAll(this);
		InputDelegates->OnJumpPressed.RemoveAll(this);
		InputDelegates->OnJumpReleased.RemoveAll(this);
	}
}

void AGPCharacter::HandleMoveInput(const FVector2D& MovementVector)
{
	if (PlayerController == nullptr)
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
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->AddYawInput(LookAxisVector.X);
	PlayerController->AddPitchInput(LookAxisVector.Y);
}

void AGPCharacter::HandleJumpPressedInput()
{
	Jump();
}

void AGPCharacter::HandleJumpReleasedInput()
{
	StopJumping();
}

// End input delegate handlers

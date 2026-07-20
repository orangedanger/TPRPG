#include "Gameplay/Character/GPCharacter.h"

#include "Framework/Input/GFInputManager.h"
#include "Gameplay/Abilities/GA_BasicAttack.h"
#include "Gameplay/Attributes/GPHealthAttributeSet.h"
#include "Gameplay/Components/GPCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/PlayerController/GPPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPCharacter, Log, All);

AGPCharacter::AGPCharacter()
{
	CombatComponent = CreateDefaultSubobject<UGPCombatComponent>(TEXT("CombatComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	HealthAttributeSet = CreateDefaultSubobject<UGPHealthAttributeSet>(TEXT("HealthAttributeSet"));
}

// Begin AActor interface
void AGPCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeAbilityActorInfo();
	GrantStartupAbilities();
}

void AGPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
 * IAbilitySystemInterface Begin
 */
UAbilitySystemComponent* AGPCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
/**
 * IAbilitySystemInterface End
 */

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

	if (IsCharacterDead())
	{
		UE_LOG(LogGPCharacter, Log, TEXT("TakeDamage skipped because receiver is already dead. Receiver=%s Instigator=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), DamageAmount);
		return;
	}

	if (DamageAmount <= 0.0f)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped because damage is invalid. Receiver=%s Instigator=%s Damage=%.2f"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), DamageAmount);
		return;
	}

	if (AbilitySystemComponent == nullptr || HealthAttributeSet == nullptr)
	{
		UE_LOG(LogGPCharacter, Warning, TEXT("TakeDamage skipped because GAS Health is unavailable. Receiver=%s Instigator=%s Causer=%s Damage=%.2f ASC=%s HealthAttributeSet=%s"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount, *GetNameSafe(AbilitySystemComponent), *GetNameSafe(HealthAttributeSet));
		return;
	}

	const float OldHealth = HealthAttributeSet->GetHealth();
	// 服务器只修改 GAS Health；旧属性组件不参与结算或死亡，避免双写和重复死亡表现。
	AbilitySystemComponent->ApplyModToAttribute(UGPHealthAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -DamageAmount);

	const float NewHealth = HealthAttributeSet->GetHealth();
	UE_LOG(LogGPCharacter, Log, TEXT("TakeDamage. Receiver=%s Instigator=%s Causer=%s Damage=%.2f OldHealth=%.2f NewHealth=%.2f Impact=%s"), *GetNameSafe(this), *GetNameSafe(DamageInstigator), *GetNameSafe(DamageCauser), DamageAmount, OldHealth, NewHealth, *HitResult.ImpactPoint.ToString());
	if (NewHealth <= 0.0f)
	{
		ApplyDeathRagdoll();
	}
}

void AGPCharacter::HandleHealthDepleted()
{
	if (HasAuthority() == false || bDeathHandled || IsCharacterDead() == false)
	{
		return;
	}

	// Health 已由 GameplayEffect 结算为零；表现层沿用现有布娃娃，不重复处理后续攻击。
	bDeathHandled = true;
	ApplyDeathRagdoll();
}
/**
 * IDamageManagerInterface End
 */

void AGPCharacter::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	// Owner、Avatar 都是角色自身，不依赖 Controller 或 PlayerState，因此 BeginPlay 初始化一次即可。
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void AGPCharacter::GrantStartupAbilities()
{
	if (HasAuthority() == false || AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 能力规格只能由服务器授予；本地输入随后请求 ASC 激活，避免 Character 直接调用战斗组件。
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_BasicAttack::StaticClass(), 1, INDEX_NONE, this));
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
/**
 * 输入委托处理 Begin
 */
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
		return;
	}

	UGFInputManager* CurrentInputManager = CurrentPlayerController->GetInputManager();
	if (CurrentInputManager == nullptr)
	{
		return;
	}

	FGFInputDelegates& InputDelegates = CurrentInputManager->GetInputDelegates();
	InputDelegates.OnMoveInput.RemoveAll(this);
	InputDelegates.OnLookInput.RemoveAll(this);
	InputDelegates.OnJumpPressed.RemoveAll(this);
	InputDelegates.OnJumpReleased.RemoveAll(this);
	InputDelegates.OnAttackPressed.RemoveAll(this);
	InputDelegates.OnAttackReleased.RemoveAll(this);

	InputDelegates.OnMoveInput.AddUObject(this, &AGPCharacter::HandleMoveInput);
	InputDelegates.OnLookInput.AddUObject(this, &AGPCharacter::HandleLookInput);
	InputDelegates.OnJumpPressed.AddUObject(this, &AGPCharacter::HandleJumpPressedInput);
	InputDelegates.OnJumpReleased.AddUObject(this, &AGPCharacter::HandleJumpReleasedInput);
	InputDelegates.OnAttackPressed.AddUObject(this, &AGPCharacter::HandleAttackPressedInput);
}

void AGPCharacter::ClearInputDelegateBindings()
{
	AGPPlayerController* CurrentPlayerController = Cast<AGPPlayerController>(Controller);
	if (CurrentPlayerController == nullptr || CurrentPlayerController->IsLocalController() == false)
	{
		return;
	}

	UGFInputManager* CurrentInputManager = CurrentPlayerController->GetInputManager();
	if (CurrentInputManager == nullptr)
	{
		return;
	}

	FGFInputDelegates& InputDelegates = CurrentInputManager->GetInputDelegates();
	InputDelegates.OnMoveInput.RemoveAll(this);
	InputDelegates.OnLookInput.RemoveAll(this);
	InputDelegates.OnJumpPressed.RemoveAll(this);
	InputDelegates.OnJumpReleased.RemoveAll(this);
	InputDelegates.OnAttackPressed.RemoveAll(this);
	InputDelegates.OnAttackReleased.RemoveAll(this);
}

bool AGPCharacter::IsCharacterDead() const
{
	return HealthAttributeSet != nullptr && HealthAttributeSet->GetHealth() <= 0.0f;
}

void AGPCharacter::HandleMoveInput(const FVector2D& MovementVector)
{
	AGPPlayerController* CurrentPlayerController = Cast<AGPPlayerController>(Controller);
	if (CurrentPlayerController == nullptr || IsCharacterDead())
	{
		return;
	}

	const FRotator ControlRotation = CurrentPlayerController->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AGPCharacter::HandleLookInput(const FVector2D& LookAxisVector)
{
	AGPPlayerController* CurrentPlayerController = Cast<AGPPlayerController>(Controller);
	if (CurrentPlayerController == nullptr || IsCharacterDead())
	{
		return;
	}

	CurrentPlayerController->AddYawInput(LookAxisVector.X);
	CurrentPlayerController->AddPitchInput(LookAxisVector.Y);
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
	if (IsCharacterDead())
	{
		return;
	}

	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 输入层只发起 GAS 激活请求，具体 Sweep 和伤害规则由基础攻击 Ability 处理。
	AbilitySystemComponent->TryActivateAbilityByClass(UGA_BasicAttack::StaticClass());
}

/**
 * 输入委托处理 End
 */

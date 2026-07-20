#include "Gameplay/Components/GPCombatComponent.h"

#include "CollisionShape.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Gameplay/Components/GPAttributeSetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPCombatComponent, Log, All);

static TAutoConsoleVariable<int32> CVarGPCombatDrawAttackDebug(
	TEXT("gp.Combat.DrawAttackDebug"),
	0,
	TEXT("Draw GP combat attack sweep debug. 0: off, 1: on."),
	ECVF_Cheat);

UGPCombatComponent::UGPCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGPCombatComponent::HandleAttackInput()
{
	if (IsOwnerDead())
	{
		UE_LOG(LogGPCombatComponent, Log, TEXT("Attack ignored because owner is dead. Owner=%s"), *GetNameSafe(GetOwner()));
		return;
	}

	const AActor* OwnerActor = GetOwner();
	if (OwnerActor != nullptr && OwnerActor->HasAuthority())
	{
		PerformAttackSweep();
		return;
	}

	ServerHandleAttackInput();
}

void UGPCombatComponent::ResolveBasicAttackConfig(float& OutDamage, float& OutRange, float& OutSweepRadius, float& OutCooldown, FName& OutSkillId) const
{
	OutDamage = BaseDamage;
	OutRange = AttackRange;
	OutSweepRadius = AttackSweepRadius;
	OutCooldown = 0.0f;
	OutSkillId = TEXT("FallbackBasicAttack");

	const FGPSkillRow* SkillRow = BasicAttackRow.GetRow<FGPSkillRow>(TEXT("UGPCombatComponent::ResolveBasicAttackConfig"));
	if (SkillRow == nullptr)
	{
		UE_LOG(LogGPCombatComponent, Warning, TEXT("BasicAttackRow is not configured or row is missing. Owner=%s Table=%s Row=%s FallbackDamage=%.2f FallbackRange=%.2f FallbackRadius=%.2f"), *GetNameSafe(GetOwner()), *GetNameSafe(BasicAttackRow.DataTable), *BasicAttackRow.RowName.ToString(), OutDamage, OutRange, OutSweepRadius);
		return;
	}

	OutDamage = SkillRow->Damage;
	OutRange = SkillRow->Range;
	OutSweepRadius = SkillRow->SweepRadius;
	OutCooldown = SkillRow->Cooldown;
	OutSkillId = SkillRow->SkillId.IsNone() ? BasicAttackRow.RowName : SkillRow->SkillId;
	UE_LOG(LogGPCombatComponent, Log, TEXT("BasicAttackRow resolved. Owner=%s SkillId=%s Table=%s Row=%s Damage=%.2f Range=%.2f Radius=%.2f Cooldown=%.2f"), *GetNameSafe(GetOwner()), *OutSkillId.ToString(), *GetNameSafe(BasicAttackRow.DataTable), *BasicAttackRow.RowName.ToString(), OutDamage, OutRange, OutSweepRadius, OutCooldown);
}

FVector UGPCombatComponent::GetAttackDirection() const
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	const APlayerController* PlayerController = nullptr;
	if (OwnerPawn != nullptr)
	{
		PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	}

	FVector MouseWorldLocation = FVector::ZeroVector;
	FVector MouseWorldDirection = FVector::ZeroVector;
	if (PlayerController != nullptr && PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
	{
		return MouseWorldDirection.GetSafeNormal();
	}

	if (PlayerController != nullptr)
	{
		return PlayerController->GetControlRotation().Vector().GetSafeNormal();
	}

	return OwnerActor != nullptr ? OwnerActor->GetActorForwardVector() : FVector::ForwardVector;
}

FGPAttackSweepResult UGPCombatComponent::PerformAttackSweep()
{
	FGPAttackSweepResult AttackSweepResult;

	if (IsOwnerDead())
	{
		UE_LOG(LogGPCombatComponent, Log, TEXT("Attack sweep skipped because owner is dead. Owner=%s"), *GetNameSafe(GetOwner()));
		return AttackSweepResult;
	}

	float ResolvedDamage = BaseDamage;
	float ResolvedRange = AttackRange;
	float ResolvedSweepRadius = AttackSweepRadius;
	float ResolvedCooldown = 0.0f;
	FName ResolvedSkillId = NAME_None;
	ResolveBasicAttackConfig(ResolvedDamage, ResolvedRange, ResolvedSweepRadius, ResolvedCooldown, ResolvedSkillId);
	UE_LOG(LogGPCombatComponent, Log, TEXT("Attack requested. Owner=%s SkillId=%s Damage=%.2f Range=%.2f Radius=%.2f Cooldown=%.2f"), *GetNameSafe(GetOwner()), *ResolvedSkillId.ToString(), ResolvedDamage, ResolvedRange, ResolvedSweepRadius, ResolvedCooldown);

	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (OwnerActor == nullptr || World == nullptr)
	{
		UE_LOG(LogGPCombatComponent, Warning, TEXT("Attack sweep skipped. Owner=%s World=%s"), *GetNameSafe(OwnerActor), *GetNameSafe(World));
		return AttackSweepResult;
	}

	const FVector Start = OwnerActor->GetActorLocation();
	const FVector AttackDirection = GetAttackDirection();
	const FVector SafeAttackDirection = AttackDirection.IsNearlyZero() ? OwnerActor->GetActorForwardVector() : AttackDirection.GetSafeNormal();
	const FVector End = Start + SafeAttackDirection * ResolvedRange;
	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(ResolvedSweepRadius);
	UE_LOG(LogGPCombatComponent, Log, TEXT("Attack sweep started. Owner=%s SkillId=%s Start=%s End=%s Radius=%.2f Channel=%d"), *GetNameSafe(OwnerActor), *ResolvedSkillId.ToString(), *Start.ToString(), *End.ToString(), ResolvedSweepRadius, static_cast<int32>(AttackTraceChannel.GetValue()));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GPCombatAttackSweep), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		AttackTraceChannel,
		SweepShape,
		QueryParams);

	DrawAttackDebug(Start, End, ResolvedSweepRadius, HitResult, bHit);

	AActor* HitActor = HitResult.GetActor();
	if (!bHit || HitActor == nullptr || HitActor == OwnerActor)
	{
		UE_LOG(LogGPCombatComponent, Log, TEXT("Attack sweep missed. Owner=%s bHit=%s HitActor=%s"), *GetNameSafe(OwnerActor), bHit ? TEXT("true") : TEXT("false"), *GetNameSafe(HitActor));
		return AttackSweepResult;
	}

	UE_LOG(LogGPCombatComponent, Log, TEXT("Attack sweep hit. Owner=%s SkillId=%s HitActor=%s Impact=%s Damage=%.2f"), *GetNameSafe(OwnerActor), *ResolvedSkillId.ToString(), *GetNameSafe(HitActor), *HitResult.ImpactPoint.ToString(), ResolvedDamage);

	AttackSweepResult.bHit = true;
	AttackSweepResult.HitActor = HitActor;
	AttackSweepResult.HitResult = HitResult;
	AttackSweepResult.Damage = ResolvedDamage;
	return AttackSweepResult;
}

void UGPCombatComponent::ServerHandleAttackInput_Implementation()
{
	PerformAttackSweep();
}

bool UGPCombatComponent::ServerHandleAttackInput_Validate()
{
	return true;
}

void UGPCombatComponent::DrawAttackDebug(const FVector& Start, const FVector& End, float SweepRadius, const FHitResult& HitResult, bool bHit) const
{
#if ENABLE_DRAW_DEBUG
	if (CVarGPCombatDrawAttackDebug.GetValueOnGameThread() <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
	DrawDebugLine(World, Start, End, DebugColor, false, 1.0f, 0, 2.0f);
	DrawDebugSphere(World, bHit ? HitResult.ImpactPoint : End, SweepRadius, 16, DebugColor, false, 1.0f);
#endif
}

bool UGPCombatComponent::IsOwnerDead() const
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return true;
	}

	const UGPAttributeSetComponent* AttributeSet = OwnerActor->FindComponentByClass<UGPAttributeSetComponent>();
	return AttributeSet != nullptr && AttributeSet->IsDead();
}

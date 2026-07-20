#include "Gameplay/Abilities/GA_BasicAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Gameplay/Components/GPCombatComponent.h"
#include "Gameplay/Effects/GE_Damage.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

void UGA_BasicAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AActor* AvatarActor = ActorInfo != nullptr ? ActorInfo->AvatarActor.Get() : nullptr;
	UGPCombatComponent* CombatComponent = AvatarActor != nullptr ? AvatarActor->FindComponentByClass<UGPCombatComponent>() : nullptr;
	if (CombatComponent == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Ability 负责把攻击检测结果转换为 GAS 伤害效果；Sweep 规则仍由战斗组件统一维护。
	const FGPAttackSweepResult AttackResult = CombatComponent->PerformAttackSweep();
	if (!AttackResult.bHit)
	{
		// 未命中属于一次正常攻击结束，不取消已成功激活的 Ability。
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	IAbilitySystemInterface* SourceAbilitySystemInterface = Cast<IAbilitySystemInterface>(AvatarActor);
	IAbilitySystemInterface* TargetAbilitySystemInterface = Cast<IAbilitySystemInterface>(AttackResult.HitActor);
	UAbilitySystemComponent* SourceAbilitySystemComponent = SourceAbilitySystemInterface != nullptr ? SourceAbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	UAbilitySystemComponent* TargetAbilitySystemComponent = TargetAbilitySystemInterface != nullptr ? TargetAbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (SourceAbilitySystemComponent == nullptr || TargetAbilitySystemComponent == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayTag HealthDeltaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.HealthDelta")), false);
	if (!HealthDeltaTag.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(AvatarActor);
	FGameplayEffectSpecHandle DamageEffectSpec = SourceAbilitySystemComponent->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.0f, EffectContext);
	if (!DamageEffectSpec.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	DamageEffectSpec.Data.Get()->SetSetByCallerMagnitude(HealthDeltaTag, -AttackResult.Damage);
	SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec.Data.Get(), TargetAbilitySystemComponent);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

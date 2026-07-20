#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/GPGameplayAbility.h"
#include "GA_BasicAttack.generated.h"

/**
 * 基础攻击 Gameplay Ability。
 *
 * 激活时复用 Avatar 上战斗组件的 Sweep 与伤害结算，避免 Ability 和战斗规则重复实现。
 */
UCLASS()
class GPGAMEPLAY_API UGA_BasicAttack : public UGPGameplayAbility
{
	GENERATED_BODY()

protected:
	/** 激活后执行一次基础攻击 Sweep，并立即结束本次 Ability。 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GPGameplayAbility.generated.h"

/**
 * GPGameplay 的通用 Gameplay Ability 基类。
 *
 * 每个 Avatar 持有独立实例，供需要保存角色级运行时状态的具体技能复用。
 */
UCLASS(Abstract)
class GPGAMEPLAY_API UGPGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGPGameplayAbility();
};

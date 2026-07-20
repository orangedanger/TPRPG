#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

/**
 * 即时结算生命值变化的 GameplayEffect。
 *
 * 调用方通过 SetByCaller 标签 Data.HealthDelta 传入生命变化量；
 * 负值表示伤害，正值可用于复用同一结算路径恢复生命。
 */
UCLASS()
class GPGAMEPLAY_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage();
};

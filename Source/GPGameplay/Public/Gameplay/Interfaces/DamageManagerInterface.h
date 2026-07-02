#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageManagerInterface.generated.h"

/** 标记对象可以作为伤害发起者或伤害接收者参与当前 Demo 的伤害流程。 */
UINTERFACE(MinimalAPI)
class UDamageManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 当前 Demo 使用的最小伤害接口。
 * `MakeDamage` 由攻击发起者调用，内部再把伤害转交给目标的 `TakeDamage`。
 */
class GPGAMEPLAY_API IDamageManagerInterface
{
	GENERATED_BODY()

public:
	/** 向目标发起伤害请求，调用方负责传入本次命中的目标和命中信息。 */
	virtual void MakeDamage(AActor* TargetActor, float DamageAmount, const FHitResult& HitResult) = 0;

	/** 接收伤害并执行扣血、死亡等结果处理。 */
	virtual void TakeDamage(AActor* DamageInstigator, AActor* DamageCauser, float DamageAmount, const FHitResult& HitResult) = 0;
};

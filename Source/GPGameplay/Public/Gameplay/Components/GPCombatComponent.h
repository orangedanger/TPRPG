#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GPCombatComponent.generated.h"

/**
 * 玩家战斗组件，负责执行当前 Demo 的临时命中检测与伤害请求。
 * 该组件不直接依赖输入系统，由角色或其他玩法对象在合适时机调用攻击入口。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GPGAMEPLAY_API UGPCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGPCombatComponent();

	/** 执行一次普通攻击，通常由角色响应输入委托后调用。 */
	void HandleAttackInput();

protected:
	/** 单次普通攻击造成的临时伤害值，后续可由技能表或武器配置替代。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** 从角色位置朝鼠标方向检测的最大距离。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat", meta = (ClampMin = "0.0"))
	float AttackRange = 300.0f;

	/** 临时 Sweep 使用的球体半径，提供一点近战命中容错。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat", meta = (ClampMin = "0.0"))
	float AttackSweepRadius = 50.0f;

	/** 临时攻击检测使用的碰撞通道，默认寻找 Pawn。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat")
	TEnumAsByte<ECollisionChannel> AttackTraceChannel = ECC_Pawn;

private:
	/** 根据玩家控制器和鼠标位置计算攻击方向，失败时回退到 Owner 朝向。 */
	FVector GetAttackDirection() const;

	/** 执行临时 Sweep 检测，并把命中的目标交给伤害接口。 */
	void PerformAttackSweep();

	/** 当 Debug CVar 开启时绘制攻击 Sweep 的调试图形。 */
	void DrawAttackDebug(const FVector& Start, const FVector& End, const FHitResult& HitResult, bool bHit) const;
};

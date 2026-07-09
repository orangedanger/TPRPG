#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Gameplay/Data/GPSkillData.h"
#include "GPCombatComponent.generated.h"

/**
 * 玩家战斗组件，负责执行当前 Demo 的临时命中检测与伤害请求。
 * 输入由角色接收后转发到本组件，本组件只处理攻击规则、命中检测和伤害请求。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GPGAMEPLAY_API UGPCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGPCombatComponent();

	/** 执行一次普通攻击，通常由本组件响应攻击输入委托后调用。 */
	void HandleAttackInput();

protected:
	/** 单次普通攻击造成的临时伤害值，后续可由技能表或武器配置替代。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** 普通攻击技能行，配置后优先使用 DataTable 中的数值；未配置时回退到本组件默认数值。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GP|Combat")
	FDataTableRowHandle BasicAttackRow;

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
	/** 解析普通攻击配置；DataTable 未配置或行无效时使用组件上的 fallback 数值。 */
	void ResolveBasicAttackConfig(float& OutDamage, float& OutRange, float& OutSweepRadius, float& OutCooldown, FName& OutSkillId) const;

	/** 根据玩家控制器和鼠标位置计算攻击方向，失败时回退到 Owner 朝向。 */
	FVector GetAttackDirection() const;

	/** 在服务器权威侧执行 Sweep 检测，并把命中的目标交给伤害接口。 */
	void PerformAttackSweep();

	/** 接收拥有者客户端的攻击请求，服务器使用自身状态重新执行命中检测和伤害结算。 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleAttackInput();

	/** 当 Debug CVar 开启时绘制攻击 Sweep 的调试图形。 */
	void DrawAttackDebug(const FVector& Start, const FVector& End, float SweepRadius, const FHitResult& HitResult, bool bHit) const;

	/** 查询 Owner 是否已死亡，死亡后不再响应攻击输入。 */
	bool IsOwnerDead() const;
};

#pragma once

#include "CoreMinimal.h"
#include "Framework/Character/GFCharacter.h"
#include "Gameplay/Interfaces/DamageManagerInterface.h"
#include "GPCharacter.generated.h"

class UGPAttributeSetComponent;
class UGPCombatComponent;
class UInputManager;

/**
 * 项目默认角色，负责组合属性与战斗组件，并作为当前 Demo 的输入响应者、伤害发起者和伤害接收者。
 * 输入管理器只广播输入意图，角色订阅后把移动、镜头、跳跃和攻击分发到对应玩法实现。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPCharacter : public AGFCharacter, public IDamageManagerInterface
{
	GENERATED_BODY()

public:
	AGPCharacter();

	/**
	 * IDamageManagerInterface Begin
	 */

	/** 向命中目标发起伤害，目标需要实现 `IDamageManagerInterface` 才会接收。 */
	virtual void MakeDamage(AActor* TargetActor, float DamageAmount, const FHitResult& HitResult) override;

	/** 接收伤害并转交给属性组件扣减生命值。 */
	virtual void TakeDamage(AActor* DamageInstigator, AActor* DamageCauser, float DamageAmount, const FHitResult& HitResult) override;

	/**
	 * IDamageManagerInterface End
	 */

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;

	/** 角色生命值和死亡状态的最小属性组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Attribute")
	TObjectPtr<UGPAttributeSetComponent> AttributeSetComponent;

	/** 角色普通攻击、命中检测和伤害发起的战斗组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Combat")
	TObjectPtr<UGPCombatComponent> CombatComponent;

private:
	/** 绑定当前控制器上的输入管理器委托，适用于 BeginPlay 和 Possess 后刷新。 */
	void RefreshInputBinding();

	/** 清理输入委托绑定，避免控制器切换或销毁后留下悬挂回调。 */
	void ClearInputBinding();

	/** 响应移动输入委托，按控制器 Yaw 将二维输入转换为世界移动。 */
	void HandleMoveInput(const FVector2D& MovementVector);

	/** 响应镜头输入委托，把二维视角轴转交给控制器。 */
	void HandleLookInput(const FVector2D& LookAxisVector);

	/** 响应跳跃输入委托。 */
	void HandleJumpInput();

	/** 响应攻击输入委托，并把战斗行为转交给 CombatComponent。 */
	void HandleAttackInput();

	/** 当前已经绑定过的输入管理器，用于控制器切换或销毁时从正确实例清理回调。 */
	UPROPERTY(Transient)
	TObjectPtr<UInputManager> BoundInputManager = nullptr;

};

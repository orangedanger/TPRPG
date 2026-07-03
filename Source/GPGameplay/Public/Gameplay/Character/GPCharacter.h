#pragma once

#include "CoreMinimal.h"
#include "Framework/Character/GFCharacter.h"
#include "Gameplay/Interfaces/DamageManagerInterface.h"
#include "GPCharacter.generated.h"

class UGPAttributeSetComponent;
class UGPCombatComponent;
class AGPPlayerController;

/**
 * 项目默认角色，负责组合属性与战斗组件，并作为当前 Demo 的输入响应者、伤害发起者和伤害接收者。
 * 角色通过当前 PlayerController 持有的输入委托对象订阅基础角色输入，攻击输入由战斗组件自行订阅。
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

	/** 角色生命值和死亡状态的最小属性组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Attribute")
	TObjectPtr<UGPAttributeSetComponent> AttributeSetComponent;

	/** 角色普通攻击、命中检测和伤害发起的战斗组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Combat")
	TObjectPtr<UGPCombatComponent> CombatComponent;

private:
	/** 初始化属性组件上的状态委托绑定，用于响应死亡等属性变化。 */
	void InitializeAttributeDelegateBindings();

	/** 清理属性组件委托绑定，避免销毁后继续收到组件广播。 */
	void ClearAttributeDelegateBindings();

	/** 初始化当前控制器输入委托对象上的绑定，当前 Demo 只在 BeginPlay 执行一次。 */
	void InitializeInputDelegateBindings();

	/** 清理输入委托绑定，避免销毁后留下悬挂回调。 */
	void ClearInputDelegateBindings();

	/** 响应移动输入委托，按控制器 Yaw 将二维输入转换为世界移动。 */
	void HandleMoveInput(const FVector2D& MovementVector);

	/** 响应镜头输入委托，把二维视角轴转交给控制器。 */
	void HandleLookInput(const FVector2D& LookAxisVector);

	/** 响应跳跃按下输入委托。 */
	void HandleJumpPressedInput();

	/** 响应跳跃松开输入委托。 */
	void HandleJumpReleasedInput();

	/** 响应属性组件死亡广播，当前只触发角色本地死亡表现。 */
	UFUNCTION()
	void HandleAttributeOwnerDead(AActor* DeadActor);

	// TODO: 临时死亡效果
	/** 让 Mesh 进入布娃娃并关闭 Capsule 碰撞，后续正式死亡流程会替换这里。 */
	void ApplyDeathRagdoll();
};

#pragma once

#include "CoreMinimal.h"
#include "Framework/Character/GFCharacter.h"
#include "Gameplay/Interfaces/DamageManagerInterface.h"
#include "GPCharacter.generated.h"

class UGPAttributeSetComponent;
class UGPCombatComponent;

/**
 * 项目默认角色，负责组合属性与战斗组件，并作为当前 Demo 的输入响应者、伤害发起者和伤害接收者。
 * 角色通过当前 PlayerController 的 InputManager 订阅输入，并把攻击输入转发给战斗组件。
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
	/** BeginPlay 时只初始化属性委托；输入绑定由 Possess 或 Controller 复制驱动。 */
	virtual void BeginPlay() override;

	/** EndPlay 时清理属性和输入委托，避免对象销毁后继续收到广播。 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 服务端 Possess 后重建角色和战斗组件输入委托绑定。 */
	virtual void PossessedBy(AController* NewController) override;

	/** 服务端 UnPossess 前清理当前控制器上的输入委托绑定。 */
	virtual void UnPossessed() override;

	/** 客户端收到 Controller 复制后重建输入委托绑定。 */
	virtual void OnRep_Controller() override;

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

	/** 从当前本地控制器的 InputManager 绑定输入委托，重复调用时先清理自身旧绑定。 */
	void BindInputDelegateBindings();

	/** 从当前本地控制器的 InputManager 清理自身输入委托。 */
	void ClearInputDelegateBindings();

	/** 死亡后屏蔽移动、跳跃和攻击等 gameplay 输入。 */
	bool IsCharacterDead() const;

	/** 响应移动输入委托，按控制器 Yaw 将二维输入转换为世界移动。 */
	void HandleMoveInput(const FVector2D& MovementVector);

	/** 响应镜头输入委托，把二维视角轴转交给控制器。 */
	void HandleLookInput(const FVector2D& LookAxisVector);

	/** 响应跳跃按下输入委托。 */
	void HandleJumpPressedInput();

	/** 响应跳跃松开输入委托。 */
	void HandleJumpReleasedInput();

	/** 响应攻击按下输入委托，并把攻击请求转交给战斗组件。 */
	void HandleAttackPressedInput();

	/** 响应属性组件死亡广播，当前只触发角色本地死亡表现。 */
	UFUNCTION()
	void HandleAttributeOwnerDead(AActor* DeadActor);

	// TODO: 临时死亡效果
	/** 让 Mesh 进入布娃娃并关闭 Capsule 碰撞，后续正式死亡流程会替换这里。 */
	void ApplyDeathRagdoll();

};

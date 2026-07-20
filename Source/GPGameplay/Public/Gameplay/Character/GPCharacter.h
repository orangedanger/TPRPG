#pragma once

#include "CoreMinimal.h"
#include "Framework/Character/GFCharacter.h"
#include "Gameplay/Interfaces/DamageManagerInterface.h"
#include "AbilitySystemInterface.h"
#include "GPCharacter.generated.h"

class UGPHealthAttributeSet;
class UGPCombatComponent;
class UAbilitySystemComponent;

/**
 * 项目默认角色，负责组合属性与战斗组件，并作为当前 Demo 的输入响应者、伤害发起者和伤害接收者。
 * 角色通过当前 PlayerController 的 InputManager 订阅输入，并由 ASC 激活攻击能力处理攻击请求。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPCharacter : public AGFCharacter, public IDamageManagerInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGPCharacter();

	/**
	 * IAbilitySystemInterface Begin
	 */

	/** 返回本角色的 ASC，供攻击者与受击目标建立 GAS 上下文。 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	 * IAbilitySystemInterface End
	 */

	/**
	 * IDamageManagerInterface Begin
	 */

	/** 向命中目标发起伤害，目标需要实现 `IDamageManagerInterface` 才会接收。 */
	virtual void MakeDamage(AActor* TargetActor, float DamageAmount, const FHitResult& HitResult) override;

	/** 在服务器通过 GAS Health 结算伤害；生命归零时触发当前临时的死亡布娃娃表现。 */
	virtual void TakeDamage(AActor* DamageInstigator, AActor* DamageCauser, float DamageAmount, const FHitResult& HitResult) override;

	/** 由 GAS Health 归零通知调用，在服务端仅触发一次当前木桩死亡表现。 */
	void HandleHealthDepleted();

	/**
	 * IDamageManagerInterface End
	 */

protected:
	/** BeginPlay 时初始化 GAS ActorInfo；输入绑定由 Possess 或 Controller 复制驱动。 */
	virtual void BeginPlay() override;

	/** EndPlay 时清理输入委托，避免对象销毁后继续收到广播。 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 服务端 Possess 后重建角色和战斗组件输入委托绑定。 */
	virtual void PossessedBy(AController* NewController) override;

	/** 服务端 UnPossess 前清理当前控制器上的输入委托绑定。 */
	virtual void UnPossessed() override;

	/** 客户端收到 Controller 复制后重建输入委托绑定。 */
	virtual void OnRep_Controller() override;

	/**
	 * GAS 生命属性真源。AGPCharacter 和 BP_TestEnemy 都继承其 100/100 默认值；
	 * 旧属性组件不再参与生命与死亡判定，避免双写和重复触发死亡。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Attribute")
	TObjectPtr<UGPHealthAttributeSet> HealthAttributeSet;

	/** 角色普通攻击、命中检测和伤害发起的战斗组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Combat")
	TObjectPtr<UGPCombatComponent> CombatComponent;

	/** 当前单机 Demo 的默认 Pawn 同时承担攻击者、受击目标，故 ASC 放在 Actor；未来若玩家需要跨重生能力状态才迁移到 PlayerState，NPC 可继续用 Actor ASC。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Ability")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

private:
	/** 初始化 GAS ActorInfo；当前角色自身同时作为 Owner 和 Avatar。 */
	void InitializeAbilityActorInfo();

	/** 防止连续 GameplayEffect 在 Health 已归零后重复触发布娃娃表现。 */
	bool bDeathHandled = false;

	/** 仅由服务器在启动时为 ASC 授予基础攻击能力，客户端通过复制接收能力状态。 */
	void GrantStartupAbilities();

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

	/** 响应攻击按下输入委托，并通过 ASC 激活基础攻击能力。 */
	void HandleAttackPressedInput();

	// TODO: 临时死亡效果
	/** 让 Mesh 进入布娃娃并关闭 Capsule 碰撞，后续正式死亡流程会替换这里。 */
	void ApplyDeathRagdoll();

};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/DelegateDefine.h"
#include "GPAttributeSetComponent.generated.h"

/**
 * 轻量角色属性组件，保存当前 Demo 需要的生命值和死亡状态。
 * 通过 SetHealth 修改生命值，避免调用方忘记同步 bDead。
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GPGAMEPLAY_API UGPAttributeSetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGPAttributeSetComponent();

	/** 设置当前生命值，并同步死亡状态。 */
	UFUNCTION(BlueprintCallable, Category = "GP|Attribute")
	void SetHealth(float NewHealth);

	/** 读取当前生命值，供战斗逻辑和蓝图显示使用。 */
	UFUNCTION(BlueprintCallable, Category = "GP|Attribute")
	float GetHealth() const;

	/** 判断生命值是否已经归零。 */
	UFUNCTION(BlueprintCallable, Category = "GP|Attribute")
	bool IsDead() const;

	/** 获取 0 到 1 的生命值比例，用于 UI 或调试显示。 */
	UFUNCTION(BlueprintCallable, Category = "GP|Attribute")
	float GetHealthPercent() const;

protected:
	virtual void BeginPlay() override;
	
public:
	/** Owner 首次进入死亡状态时广播，角色可绑定它执行死亡表现。 */
	UPROPERTY(BlueprintAssignable, Category = "GP|Attribute")
	FGFActorDeadDelegate OnOwnerDead;
private:
	/** 当前生命值，只通过 SetHealth 修改以保持状态一致。 */
	UPROPERTY()
	float Health = 100.0f;

	/** 最大生命值，用于限制当前生命和计算生命比例。 */
	UPROPERTY()
	float MaxHealth = 100.0f;

	/** 缓存死亡状态，便于角色和蓝图直接查询。 */
	UPROPERTY()
	bool bDead = false;
};

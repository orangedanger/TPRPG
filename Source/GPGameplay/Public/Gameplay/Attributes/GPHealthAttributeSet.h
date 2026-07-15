#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GPHealthAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * GAS 生命属性集，保存需要由服务端权威修改并复制到客户端的当前生命与最大生命。
 *
 * 本迁移阶段只负责属性初始值和边界约束；不处理 GameplayEffect 结算、标签逻辑或完整死亡状态机。
 */
UCLASS()
class GPGAMEPLAY_API UGPHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGPHealthAttributeSet();

	/** 当前生命值。服务端权威修改，客户端通过 ASC 属性复制接收更新。 */
	UPROPERTY(BlueprintReadOnly, Category = "GP|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGPHealthAttributeSet, Health)

	/** 最大生命值。服务端权威修改，客户端通过 ASC 属性复制接收更新。 */
	UPROPERTY(BlueprintReadOnly, Category = "GP|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UGPHealthAttributeSet, MaxHealth)

	/** 为客户端同步生命属性，并始终触发属性变更通知。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 在当前值变更前约束生命属性的可用范围。 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** 在基础值变更前约束生命属性的可用范围。 */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

protected:
	/** 客户端收到当前生命复制时，通知 GAS 刷新对应属性。 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	/** 客户端收到最大生命复制时，通知 GAS 刷新对应属性。 */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

private:
	/** 统一限制 Health 为 [0, MaxHealth]，并确保 MaxHealth 不会为负。 */
	void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;
};

#include "Gameplay/Effects/GE_Damage.h"

#include "Gameplay/Attributes/GPHealthAttributeSet.h"

UGE_Damage::UGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo& HealthModifier = Modifiers.AddDefaulted_GetRef();
	HealthModifier.Attribute = UGPHealthAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;

	// 使用标签型 SetByCaller，使每次效果规格可携带不同的生命变化量。
	FSetByCallerFloat HealthDeltaMagnitude;
	HealthDeltaMagnitude.DataTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.HealthDelta")));
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(HealthDeltaMagnitude);
}

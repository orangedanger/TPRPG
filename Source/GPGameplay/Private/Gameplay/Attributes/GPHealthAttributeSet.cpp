#include "Gameplay/Attributes/GPHealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Gameplay/Character/GPCharacter.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPHealthAttributeSet, Log, All);

UGPHealthAttributeSet::UGPHealthAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void UGPHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGPHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGPHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UGPHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttributeValue(Attribute, NewValue);
}

void UGPHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttributeValue(Attribute, NewValue);
}

void UGPHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetHealthAttribute())
	{
		return;
	}

	UE_LOG(LogGPHealthAttributeSet, Log, TEXT("Health changed by GameplayEffect. Owner=%s RequestedDelta=%.2f NewHealth=%.2f MaxHealth=%.2f"), *GetNameSafe(GetOwningActor()), Data.EvaluatedData.Magnitude, GetHealth(), GetMaxHealth());

	if (GetHealth() > 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent();
	AGPCharacter* AvatarCharacter = OwningAbilitySystemComponent != nullptr ? Cast<AGPCharacter>(OwningAbilitySystemComponent->GetAvatarActor()) : nullptr;
	if (AvatarCharacter == nullptr)
	{
		return;
	}

	// GameplayEffect 在服务端结算 Health；Character 负责一次性复用现有死亡表现。
	AvatarCharacter->HandleHealthDepleted();
}

void UGPHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPHealthAttributeSet, Health, OldHealth);
}

void UGPHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGPHealthAttributeSet, MaxHealth, OldMaxHealth);
}

void UGPHealthAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

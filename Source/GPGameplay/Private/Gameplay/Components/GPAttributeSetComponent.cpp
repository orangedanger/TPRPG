#include "Gameplay/Components/GPAttributeSetComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPAttributeSetComponent, Log, All);

UGPAttributeSetComponent::UGPAttributeSetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGPAttributeSetComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Health initialized. Owner=%s Health=%.2f MaxHealth=%.2f Dead=%s"), *GetNameSafe(GetOwner()), Health, MaxHealth, bDead ? TEXT("true") : TEXT("false"));
}

void UGPAttributeSetComponent::SetHealth(float NewHealth)
{
	const float ClampedMaxHealth = FMath::Max(0.0f, MaxHealth);
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.0f, ClampedMaxHealth);
	UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Health changed. Owner=%s OldHealth=%.2f NewHealth=%.2f RequestedHealth=%.2f MaxHealth=%.2f"), *GetNameSafe(GetOwner()), OldHealth, Health, NewHealth, ClampedMaxHealth);

	if (Health <= 0.0f)
	{
		UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Actor dead. Owner=%s"), *GetNameSafe(GetOwner()));
		bDead = true;
		OnActorDead.Broadcast(GetOwner());
	}
}

float UGPAttributeSetComponent::GetHealth() const
{
	return Health;
}

bool UGPAttributeSetComponent::IsDead() const
{
	return bDead;
}

float UGPAttributeSetComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
}

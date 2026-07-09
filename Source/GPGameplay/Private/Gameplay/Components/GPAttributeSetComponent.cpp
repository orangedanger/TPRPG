#include "Gameplay/Components/GPAttributeSetComponent.h"

#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogGPAttributeSetComponent, Log, All);

UGPAttributeSetComponent::UGPAttributeSetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGPAttributeSetComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Health initialized. Owner=%s Health=%.2f MaxHealth=%.2f Dead=%s"), *GetNameSafe(GetOwner()), Health, MaxHealth, bDead ? TEXT("true") : TEXT("false"));
}

void UGPAttributeSetComponent::SetHealth(float NewHealth)
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor != nullptr && OwnerActor->HasAuthority() == false)
	{
		UE_LOG(LogGPAttributeSetComponent, Warning, TEXT("SetHealth ignored on non-authority. Owner=%s RequestedHealth=%.2f"), *GetNameSafe(OwnerActor), NewHealth);
		return;
	}

	const float ClampedMaxHealth = FMath::Max(0.0f, MaxHealth);
	const float OldHealth = Health;
	const bool bWasDead = bDead;
	Health = FMath::Clamp(NewHealth, 0.0f, ClampedMaxHealth);
	bDead = Health <= 0.0f;
	UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Health changed. Owner=%s OldHealth=%.2f NewHealth=%.2f RequestedHealth=%.2f MaxHealth=%.2f"), *GetNameSafe(GetOwner()), OldHealth, Health, NewHealth, ClampedMaxHealth);

	BroadcastOwnerDeadIfNeeded(bWasDead);
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

void UGPAttributeSetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGPAttributeSetComponent, Health);
	DOREPLIFETIME(UGPAttributeSetComponent, MaxHealth);
	DOREPLIFETIME(UGPAttributeSetComponent, bDead);
}

void UGPAttributeSetComponent::OnRep_Health(float PreviousHealth)
{
	UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Health replicated. Owner=%s OldHealth=%.2f NewHealth=%.2f MaxHealth=%.2f"), *GetNameSafe(GetOwner()), PreviousHealth, Health, MaxHealth);
}

void UGPAttributeSetComponent::OnRep_Dead(bool bWasDead)
{
	BroadcastOwnerDeadIfNeeded(bWasDead);
}

void UGPAttributeSetComponent::BroadcastOwnerDeadIfNeeded(bool bWasDead)
{
	if (bWasDead == false && bDead)
	{
		UE_LOG(LogGPAttributeSetComponent, Log, TEXT("Actor dead. Owner=%s"), *GetNameSafe(GetOwner()));
		OnOwnerDead.Broadcast(GetOwner());
	}
}

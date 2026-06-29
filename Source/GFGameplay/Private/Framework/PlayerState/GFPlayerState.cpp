#include "Framework/PlayerState/GFPlayerState.h"

#include "Net/UnrealNetwork.h"

AGFPlayerState::AGFPlayerState()
{
	bReplicates = true;
}

int32 AGFPlayerState::GetCharacterLevel() const
{
	return CharacterLevel;
}

void AGFPlayerState::SetCharacterLevel(int32 NewLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	CharacterLevel = FMath::Max(1, NewLevel);
	OnRep_CharacterLevel();
}

void AGFPlayerState::OnRep_CharacterLevel()
{
}

void AGFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGFPlayerState, CharacterLevel);
}

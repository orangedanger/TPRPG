#pragma once

#include "CoreMinimal.h"
#include "Framework/PlayerState/GFPlayerState.h"
#include "GPPlayerState.generated.h"

UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPPlayerState : public AGFPlayerState
{
	GENERATED_BODY()

public:
	AGPPlayerState();
};

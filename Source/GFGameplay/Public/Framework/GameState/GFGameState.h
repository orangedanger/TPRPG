#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GFGameState.generated.h"

UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGFGameState();
};

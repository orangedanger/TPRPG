#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GFGameMode.generated.h"

UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGFGameMode();

protected:
	virtual void InitGameState() override;
};

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/GFGameMode.h"
#include "GPGameMode.generated.h"

UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPGameMode : public AGFGameMode
{
	GENERATED_BODY()

public:
	AGPGameMode();

protected:
	virtual void InitGameState() override;
};

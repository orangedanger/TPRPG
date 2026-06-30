#pragma once

#include "CoreMinimal.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "GPPlayerController.generated.h"

/**
 * Gameplay player controller for project-specific input manager configuration.
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPPlayerController : public AGFPlayerController
{
	GENERATED_BODY()

public:
	AGPPlayerController();
	

};

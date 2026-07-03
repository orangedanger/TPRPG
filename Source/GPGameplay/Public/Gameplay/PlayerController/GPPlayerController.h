#pragma once

#include "CoreMinimal.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "GPPlayerController.generated.h"

/**
 * 项目玩法 PlayerController，目前复用 GF 层输入初始化和输入管理器。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPPlayerController : public AGFPlayerController
{
	GENERATED_BODY()

public:
	AGPPlayerController();
	

};

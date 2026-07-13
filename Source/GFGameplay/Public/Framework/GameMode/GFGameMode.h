#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GFGameMode.generated.h"

/**
 * Gameplay Framework 的服务端规则基类。
 * GameMode 仅在 authority 存在，具体默认类和玩法规则由项目层派生类配置。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGFGameMode();
};

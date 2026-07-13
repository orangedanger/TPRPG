#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/GFGameMode.h"
#include "GPGameMode.generated.h"

/**
 * 项目的服务端规则入口，负责指定默认 Character、Controller、GameState 和 PlayerState 类型。
 * 具体关卡规则可继续在原生或 Blueprint 派生 GameMode 中扩展。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPGameMode : public AGFGameMode
{
	GENERATED_BODY()

public:
	AGPGameMode();
};

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameState/GFGameState.h"
#include "GPGameState.generated.h"

/**
 * 项目的全局复制状态扩展点。
 * 当前复用 GF 基础能力，后续全局玩法状态应由服务端写入并复制给客户端。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPGameState : public AGFGameState
{
	GENERATED_BODY()

public:
	AGPGameState();
};

#pragma once

#include "CoreMinimal.h"
#include "Framework/PlayerState/GFPlayerState.h"
#include "GPPlayerState.generated.h"

/**
 * 项目的玩家复制状态扩展点，用于承载需要跨 Pawn 保留的玩家数据。
 * 当前只继承 GF 状态，不提前声明尚未接入的能力系统运行时职责。
 */
UCLASS(Blueprintable)
class GPGAMEPLAY_API AGPPlayerState : public AGFPlayerState
{
	GENERATED_BODY()

public:
	AGPPlayerState();
};

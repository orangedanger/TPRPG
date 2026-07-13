#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GFGameState.generated.h"

/**
 * Gameplay Framework 的全局复制状态基类。
 * 服务端写入的全局比赛状态通过该对象复制给客户端，具体字段由项目层扩展。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGFGameState();
};

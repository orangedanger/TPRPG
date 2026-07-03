#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DelegateDefine.generated.h"

class AActor;

/** Actor 死亡时广播的通用委托类型，实例应挂在具体拥有者对象上。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFActorDeadDelegate, AActor*, DeadActor);

/**
 * World 级委托归口对象，后续用于承载跨系统、且生命周期属于 World 的委托实例。
 * 当前输入委托已下沉到 PlayerController 持有的输入委托对象中，避免不同玩家共享输入广播。
 */
UCLASS()
class GFGAMEPLAY_API UGFDelegateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
};

#pragma once

#include "CoreMinimal.h"
#include "DelegateDefine.generated.h"

class AActor;

/** Actor 死亡时广播的通用委托类型，实例应挂在具体拥有者对象上。 */
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGFActorDeadDelegate, AActor*, DeadActor);

/** 传递二维轴值的输入委托类型。 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGFInputVector2DDelegate, const FVector2D&);

/** 传递按下或松开事件的输入委托类型。 */
DECLARE_MULTICAST_DELEGATE(FGFInputActionDelegate);

/** 每个 InputManager 持有一份输入委托集合，避免不同本地玩家共享输入广播。 */
struct GFGAMEPLAY_API FGFInputDelegates
{
	/** 移动输入委托，参数为解析后的二维移动轴。 */
	FGFInputVector2DDelegate OnMoveInput;

	/** 镜头输入委托，参数为解析后的二维视角轴。 */
	FGFInputVector2DDelegate OnLookInput;

	/** 跳跃按下输入委托。 */
	FGFInputActionDelegate OnJumpPressed;

	/** 跳跃松开输入委托。 */
	FGFInputActionDelegate OnJumpReleased;

	/** 攻击按下输入委托。 */
	FGFInputActionDelegate OnAttackPressed;

	/** 攻击松开输入委托。 */
	FGFInputActionDelegate OnAttackReleased;
};

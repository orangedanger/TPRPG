#pragma once

#include "CoreMinimal.h"
#include "GFInputDelegates.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FGFInputVector2DDelegate, const FVector2D&);
DECLARE_MULTICAST_DELEGATE(FGFInputActionDelegate);

/**
 * 每个 PlayerController 持有一份输入委托对象，订阅者通过控制器获取并绑定自己的输入响应。
 */
USTRUCT()
struct GFGAMEPLAY_API FGFInputDelegates
{
	GENERATED_BODY()

	/** 移动输入委托，参数为已经解析后的二维移动轴。 */
	FGFInputVector2DDelegate OnMoveInput;

	/** 镜头输入委托，参数为已经解析后的二维视角轴。 */
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

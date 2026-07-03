#pragma once

#include "CoreMinimal.h"

/** 移动输入委托类型，参数为已经解析后的二维移动轴。 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMoveInput, const FVector2D&);

/** 镜头输入委托类型，参数为已经解析后的二维视角轴。 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLookInput, const FVector2D&);

/** 跳跃输入委托类型，按键类事件不携带原始输入参数。 */
DECLARE_MULTICAST_DELEGATE(FOnJumpInput);

/**
 * 攻击输入委托类型，按键类事件暂不携带原始输入参数。
 * TODO: 后续需要区分按下和释放时，考虑拆为 AttackPressed/AttackReleased 等委托，或通过统一事件结构携带 TriggerEvent。
 */
DECLARE_MULTICAST_DELEGATE(FOnAttackInput);

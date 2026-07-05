#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GPSkillData.generated.h"

/**
 * GP 技能表的第一版行结构，用于把普通攻击的数值从 CombatComponent 中抽到 DataTable。
 * 当前 Demo 只读取 BasicAttack 需要的伤害、距离、冷却和 Sweep 半径，后续再扩展动画、特效等字段。
 */
USTRUCT(BlueprintType)
struct GPGAMEPLAY_API FGPSkillRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 技能业务标识，通常与 RowName 保持一致，便于日志和调试中确认当前使用的技能。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GP|Skill")
	FName SkillId = NAME_None;

	/** 技能造成的基础伤害。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GP|Skill", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;

	/** 技能命中检测的最大距离。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GP|Skill", meta = (ClampMin = "0.0"))
	float Range = 300.0f;

	/** 技能冷却时间，第一版只记录和输出日志，暂不阻塞攻击输入。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GP|Skill", meta = (ClampMin = "0.0"))
	float Cooldown = 0.5f;

	/** Sweep 命中检测半径，用于给近战攻击提供基础容错。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GP|Skill", meta = (ClampMin = "0.0"))
	float SweepRadius = 50.0f;
};

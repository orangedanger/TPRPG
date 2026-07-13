#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GFPlayerState.generated.h"

/**
 * Gameplay Framework 的玩家复制状态基类，用于保存跨 Pawn 生命周期的玩家数据。
 * CharacterLevel 由服务端权威修改并复制到客户端，具体成长规则由项目层扩展。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AGFPlayerState();

	/** 返回当前复制的角色等级。 */
	UFUNCTION(BlueprintPure, Category = "GF|Player")
	int32 GetCharacterLevel() const;

	/** 仅在服务端修改角色等级；客户端调用不会改变权威状态。 */
	UFUNCTION(BlueprintCallable, Category = "GF|Player")
	void SetCharacterLevel(int32 NewLevel);

protected:
	/** 服务端权威角色等级，复制到客户端供成长和 UI 系统读取。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterLevel, Category = "GF|Player", meta = (ClampMin = "1"))
	int32 CharacterLevel = 1;

	/** 客户端收到 CharacterLevel 复制时的扩展点。 */
	UFUNCTION()
	virtual void OnRep_CharacterLevel();

	/** 注册 CharacterLevel 的网络复制规则。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

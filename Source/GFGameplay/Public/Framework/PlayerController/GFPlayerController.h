#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GFPlayerController.generated.h"

class UInputManager;

/**
 * 基础玩家控制器，负责创建并驱动每个本地玩家自己的输入管理器。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGFPlayerController();

	/** 返回当前控制器持有的运行时输入管理器。 */
	UFUNCTION(BlueprintPure, Category = "Input")
	UInputManager* GetInputManager() const { return InputManager; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** 蓝图可配置的输入管理器类型，通常用于挂接 MappingContext 和输入表资源。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UInputManager> InputManagerClass;
	
	/** 当前控制器拥有的运行时输入管理器实例。 */
	UPROPERTY(BlueprintReadOnly, Category = "Input", Transient)
	TObjectPtr<UInputManager> InputManager;

private:
	/** 在 BeginPlay 中创建输入管理器，并执行一次 owner 初始化。 */
	void InitializeInputManager();
};

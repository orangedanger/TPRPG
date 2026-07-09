#pragma once

#include "CoreMinimal.h"
#include "Framework/Input/GFInputDelegates.h"
#include "GameFramework/PlayerController.h"
#include "GFPlayerController.generated.h"

class UEnhancedInputComponent;
class UGFInputManager;

/**
 * 基础玩家控制器，负责所有玩法 PlayerController 共享的输入对象生命周期和复制配置。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGFPlayerController();

	/** 返回当前控制器持有的输入委托对象，订阅者通过它绑定或解绑输入响应。 */
	FGFInputDelegates* GetInputDelegates() { return &InputDelegates; }

	/** 返回当前控制器持有的输入管理器。 */
	UFUNCTION(BlueprintPure, Category = "Input")
	UGFInputManager* GetInputManager() const { return InputManager; }

protected:
	/** BeginPlay 时确保本地输入委托对象和输入管理器已经创建。 */
	virtual void BeginPlay() override;

	/** EndPlay 时移除本控制器添加的 Enhanced Input 上下文和动作绑定。 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 在引擎创建 InputComponent 后直接交给输入管理器执行 Enhanced Input 绑定。 */
	virtual void SetupInputComponent() override;

	/** 每帧驱动输入管理器处理缓存输入事件。 */
	virtual void PlayerTick(float DeltaTime) override;

	/** 蓝图可配置的输入管理器类型，玩法层通常在 C++ 构造函数中设置默认子类。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UGFInputManager> InputManagerClass;

	/** 当前控制器拥有的运行时输入管理器实例。 */
	UPROPERTY(BlueprintReadOnly, Category = "Input", Transient)
	TObjectPtr<UGFInputManager> InputManager = nullptr;

	/** 当前控制器拥有的输入委托集合，生命周期跟随控制器本身。 */
	FGFInputDelegates InputDelegates;

	/** 初始化输入委托对象和输入管理器，内部防止重复创建。 */
	void InitializeInput();

private:
	/** 已经交给输入管理器配置过的 Enhanced InputComponent，避免重复绑定同一组件。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputComponent> ConfiguredEnhancedInputComponent = nullptr;

	/** 输入对象是否已经完成创建和初始化。 */
	bool bInputInitialized = false;
};

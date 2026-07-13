#pragma once

#include "CoreMinimal.h"
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

	/** 返回当前控制器持有的输入管理器。 */
	UFUNCTION(BlueprintPure, Category = "Input")
	UGFInputManager* GetInputManager() const { return InputManager; }

protected:
	/**
	 * APlayerController Begin
	 */

	/** BeginPlay 时完成本地输入管理器初始化。 */
	virtual void BeginPlay() override;

	/** authority Possess 进入 Pawn 回调前确保本地输入管理器对象已经创建。 */
	virtual void OnPossess(APawn* InPawn) override;

	/** EndPlay 时移除本控制器添加的 Enhanced Input 上下文和动作绑定。 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 在引擎创建 InputComponent 后直接交给输入管理器执行 Enhanced Input 绑定。 */
	virtual void SetupInputComponent() override;

	/**
	 * APlayerController End
	 */

	/** 蓝图可配置的输入管理器类型，玩法层通常在 C++ 构造函数中设置默认子类。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TSubclassOf<UGFInputManager> InputManagerClass;

	/** 当前控制器拥有的运行时输入管理器实例。 */
	UPROPERTY(BlueprintReadOnly, Category = "Input", Transient)
	TObjectPtr<UGFInputManager> InputManager = nullptr;

	/** 完成本地输入管理器和 Enhanced Input 子系统初始化，内部防止重复执行。 */
	void InitializeInput();

private:
	/** 仅创建本地输入管理器对象，不提前访问 LocalPlayerSubsystem 或绑定 InputComponent。 */
	UGFInputManager* CreateInputManager();

	/** 已经交给输入管理器配置过的 Enhanced InputComponent，避免重复绑定同一组件。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputComponent> ConfiguredEnhancedInputComponent = nullptr;

	/** 输入管理器是否已经完成 Enhanced Input 子系统初始化。 */
	bool bInputInitialized = false;
};

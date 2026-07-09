#pragma once

#include "CoreMinimal.h"
#include "Framework/Input/InputDefine.h"
#include "InputActionValue.h"
#include "UObject/Object.h"
#include "GFInputManager.generated.h"

class AGFPlayerController;
class UDataTable;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

/**
 * 输入事件标签，非轴类输入先缓存标签，再在 UpdateInputActions 中统一处理。
 */
UENUM(BlueprintType)
enum class EGFInputActionTag : uint8
{
	None,
	Input_Jump_Pressed,
	Input_Jump_Released,
	Input_Attack_Pressed,
	Input_Attack_Released
};

/**
 * Gameplay Framework 输入管理器，负责 Enhanced Input 绑定、输入缓存和向控制器输入委托对象广播。
 */
UCLASS(Blueprintable, BlueprintType)
class GFGAMEPLAY_API UGFInputManager : public UObject
{
	GENERATED_BODY()

public:
	/** 设置所属 PlayerController，缓存本地玩家输入子系统，并应用配置的 MappingContext。 */
	virtual void Initialize(AGFPlayerController* InOwner);

	/** 清理由输入管理器添加的上下文和动作绑定，通常在 PlayerController 结束生命周期时调用。 */
	virtual void Deinitialize();

	/** 设置接收 Action 绑定的 Enhanced Input 组件，随后绑定 DataTable 中配置的 Action。 */
	virtual void SetInputComponent(UEnhancedInputComponent* InInputComponent);

	/** 每帧处理缓存的按键类输入标签，并广播到 Controller 持有的委托对象。 */
	virtual void UpdateInputActions(float DeltaTime);

	/** 将配置的 MappingContext 添加到初始化时缓存的 Enhanced Input 子系统。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ApplyMappingContexts();

	/** 移除当前输入管理器添加过的 MappingContext。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ClearMappingContexts();

	/** 从配置的 DataTable 重建 Action 绑定。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void BindConfiguredActions();

protected:
	/** 由输入管理器持有并在 Initialize 时应用的 MappingContext 配置。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<FInputMappingContextConfig> MappingContexts;

	/** 保存 Move、Look、Jump、Attack 等输入行配置的 DataTable。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UDataTable> InputActionTable = nullptr;

	/** Move 输入单独处理，因为它携带 FVector2D。 */
	void HandleMoveInput(const FInputActionValue& Value);

	/** Look 输入单独处理，因为它携带 FVector2D。 */
	void HandleLookInput(const FInputActionValue& Value);

	/** Jump 按下只缓存标签，实际广播在 UpdateInputActions 中处理。 */
	void HandleJumpPressedInput();

	/** Jump 松开只缓存标签，实际广播在 UpdateInputActions 中处理。 */
	void HandleJumpReleasedInput();

	/** Attack 按下只缓存标签，实际广播在 UpdateInputActions 中处理。 */
	void HandleAttackPressedInput();

	/** Attack 松开只缓存标签，实际广播在 UpdateInputActions 中处理。 */
	void HandleAttackReleasedInput();

	/** 拥有当前输入管理器的 PlayerController。 */
	UPROPERTY(Transient)
	TObjectPtr<AGFPlayerController> OwnerController = nullptr;

	/** 当前接收 Action 绑定的 Enhanced Input 组件。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = nullptr;

	/** 初始化时缓存的本地玩家输入子系统，后续函数只消费该状态，不自行重复刷新。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> LocalPlayerSubsystem = nullptr;

	/** 当前输入管理器已经添加并可安全移除的 MappingContext。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputMappingContext>> ActiveMappingContexts;

	/** 从 DataTable 加载并绑定到 Enhanced Input 的运行时记录。 */
	UPROPERTY(Transient)
	TArray<FInputActionBindingRecord> BindingRecords;

	/** 本帧等待处理的按键类输入标签。 */
	TArray<EGFInputActionTag> PendingInputTags;

private:
	void CacheLocalPlayerSubsystem();
	void AddConfiguredBindingRecord(const FInputActionTableRow& Row);
	void AddBindingRecord(EInputActionType ActionType, UInputAction* InputAction, ETriggerEvent TriggerEvent);
	void BindRecord(FInputActionBindingRecord& Record);
	void UnbindRecord(FInputActionBindingRecord& Record);
	void ClearActionBindings();
	void QueueInputAction(EGFInputActionTag InputTag);
	void ProcessInputAction(EGFInputActionTag InputTag);
	bool CanBindRecord(const FInputActionBindingRecord& Record) const;
};

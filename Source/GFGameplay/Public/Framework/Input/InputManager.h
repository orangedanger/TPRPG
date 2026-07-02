#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Framework/DelegateDefine.h"
#include "Framework/Input/InputDefine.h"
#include "UObject/Object.h"
#include "InputManager.generated.h"

class AGFPlayerController;
class UDataTable;
class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

/**
 * 负责为 PlayerController 管理 Enhanced Input 的 MappingContext 和 DataTable 驱动的 Action 绑定。
 * 输入管理器只把 Enhanced Input 转换成自身实例上的输入委托，具体玩法响应由角色或组件订阅处理。
 */
UCLASS(Blueprintable, BlueprintType)
class GFGAMEPLAY_API UInputManager : public UObject
{
	GENERATED_BODY()

public:
	/** 使用所属 PlayerController 初始化输入管理器。 */
	void Initialize(AGFPlayerController* InOwner);

	/** 设置接收 Action 绑定的 Enhanced Input 组件。 */
	void SetInputComponent(UEnhancedInputComponent* InInputComponent);

	/** 刷新用于添加 MappingContext 的本地玩家输入子系统。 */
	void RefreshLocalPlayerSubsystem();

	/** 将配置的 MappingContext 添加到本地玩家的 Enhanced Input 子系统。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ApplyMappingContexts();

	/** 移除当前输入管理器添加过的 MappingContext。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ClearMappingContexts();

	/** 从配置的 DataTable 重建 Action 绑定。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void BindConfiguredActions();

	/** 禁用指定分组下所有当前已绑定的 Action。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DisableInputGroup(EInputGroup InputGroup);

	/** 启用指定分组下所有已配置的 Action。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void EnableInputGroup(EInputGroup InputGroup);

	/** 移除所有当前生效的 Action 绑定，但保留运行时记录。 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ClearBindings();

	/** Move 输入被触发时广播，订阅者应属于同一个 PlayerController 输入上下文。 */
	FOnMoveInput OnMoveInput;

	/** Look 输入被触发时广播，订阅者应属于同一个 PlayerController 输入上下文。 */
	FOnLookInput OnLookInput;

	/** Jump 输入被触发时广播，不暴露原始 Enhanced Input 值。 */
	FOnJumpInput OnJumpInput;

	/** Attack 输入被触发时广播，不暴露原始 Enhanced Input 值。 */
	FOnAttackInput OnAttackInput;

protected:
	/** 由输入管理器持有并在初始化时应用的 MappingContext 配置。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<FInputMappingContextConfig> MappingContexts;

	/** 保存 Move、Look、Jump、Attack 等输入行配置的 DataTable。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UDataTable> InputActionTable = nullptr;

	/** Move 输入的集中处理入口。 */
	void HandleMoveInput(const FInputActionValue& Value);

	/** Look 输入的集中处理入口。 */
	void HandleLookInput(const FInputActionValue& Value);

	/** Jump 输入只广播意图，不向玩法层暴露按键输入值。 */
	void HandleJumpInput(const FInputActionValue& Value);

	/** Attack 输入只广播意图，不向玩法层暴露按键输入值。 */
	void HandleAttackInput(const FInputActionValue& Value);

	/** 拥有当前输入管理器的 PlayerController。 */
	UPROPERTY(Transient)
	TObjectPtr<AGFPlayerController> OwnerController = nullptr;

	/** 当前接收 Action 绑定的 Enhanced Input 组件。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = nullptr;

	/** 用于添加和移除 MappingContext 的本地玩家输入子系统。 */
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> LocalPlayerSubsystem = nullptr;

	/** 当前输入管理器已经添加并可安全移除的 MappingContext。 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInputMappingContext>> ActiveMappingContexts;

	/** 从 DataTable 加载并绑定到 Enhanced Input 的运行时记录。 */
	TArray<FInputActionBindingRecord> BindingRecords;

	/** 当前被玩法状态禁用的输入分组。 */
	TSet<EInputGroup> DisabledGroups;

private:
	void AddConfiguredBindingRecord(const FInputActionTableRow& Row);
	void BindRecord(FInputActionBindingRecord& Record);
	void UnbindRecord(FInputActionBindingRecord& Record);
	bool CanBindRecord(const FInputActionBindingRecord& Record) const;
};

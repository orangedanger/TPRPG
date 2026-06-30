#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputAction.h"
#include "InputDefine.generated.h"

class UInputMappingContext;

/**
 * 定义可以被统一启用或禁用的输入分组。
 */
UENUM(BlueprintType)
enum class EInputGroup : uint8
{
	None,
	Movement,
	Camera,
	Combat,
	UI,
	System
};

/**
 * 标识 InputAction 事件应该分发到哪一个输入处理函数。
 */
UENUM(BlueprintType)
enum class EInputActionType : uint8
{
	None,
	Move,
	Look,
	Jump,
	Attack
};

/**
 * 配置一个由输入管理器负责添加和移除的 MappingContext。
 */
USTRUCT(BlueprintType)
struct FInputMappingContextConfig
{
	GENERATED_BODY()

	/** 要添加到本地 Enhanced Input 子系统的 MappingContext 资源。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	/** 优先级，数值越高越先处理输入冲突。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 Priority = 0;
};

/**
 * 描述一条由 UInputManager 负责绑定的 InputAction DataTable 行。
 */
USTRUCT(BlueprintType)
struct FInputActionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 该输入事件要调用的逻辑处理类型。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EInputActionType ActionType = EInputActionType::None;

	/** 要绑定的 InputAction 资源。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** 该绑定使用的 Enhanced Input 触发事件。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;

	/** 运行时用于批量启用或禁用的输入分组。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EInputGroup InputGroup = EInputGroup::None;
};

/**
 * 保存一条从 DataTable 加载出来的输入绑定运行时状态。
 */
USTRUCT()
struct FInputActionBindingRecord
{
	GENERATED_BODY()

	/** 输入触发时要调用的逻辑处理类型。 */
	EInputActionType ActionType = EInputActionType::None;

	/** 当前绑定到 Enhanced Input 的 InputAction 资源。 */
	UPROPERTY()
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** 当前绑定使用的 Enhanced Input 触发事件。 */
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;

	/** 运行时用于批量启用或禁用的输入分组。 */
	EInputGroup InputGroup = EInputGroup::None;

	/** Enhanced Input 返回的绑定句柄，用于后续解绑。 */
	uint32 BindingHandle = 0;

	/** 当前记录是否已经存在有效的 Enhanced Input 绑定。 */
	bool bIsBound = false;
};

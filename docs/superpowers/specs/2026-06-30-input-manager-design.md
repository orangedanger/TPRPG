# InputManager 设计文档

## 背景

项目当前使用 `GFGameplay` 作为基础框架层，`GPGameplay` 作为具体玩法层。`GFPlayerController` 已经提供 `BeginPlay`、`OnPossess`、`OnRep_Pawn` 和 `SetupInputComponent` 生命周期入口，但还没有实际的 Enhanced Input 初始化与绑定管理逻辑。

本设计目标是在 `GFGameplay` 中增加一个通用 `InputManager`，用于统一管理输入初始化、`InputAction` 绑定生命周期，以及运行时按输入分组启用或禁用输入。

## 目标

- 在 `GFPlayerController` 中建立统一的输入初始化入口。
- 将 Enhanced Input 的绑定、解绑、MappingContext 管理封装到 `InputManager`。
- 支持运行时关闭或恢复某一组输入，例如移动、镜头、战斗、UI。
- 允许 `GPGameplay` 层注册具体项目的 `InputAction` 与回调。
- 保持 `GFGameplay` 框架层通用，不绑定具体玩法动作。

## 非目标

- 不在本次设计中实现完整按键重映射 UI。
- 不在 `GFGameplay` 层硬编码具体 `IA_Move`、`IA_Look`、`IA_Jump` 等资产。
- 不替代 UE Enhanced Input 的 `UEnhancedInputLocalPlayerSubsystem` 和 `UEnhancedInputComponent`。
- 不设计多人输入同步协议；输入绑定只在本地 PlayerController 侧生效。

## 推荐架构

新增 `UGFInputManager`，建议继承 `UObject`。它由 `AGFPlayerController` 创建并持有，生命周期跟随 PlayerController。

选择 `UObject` 而不是 `UActorComponent` 的原因是：`InputManager` 不需要 Tick、场景组件能力或 Actor 组件注册生命周期；它更像 PlayerController 的输入服务对象。这样可以降低运行时开销，也让职责边界更清楚。

整体关系如下：

```text
AGFPlayerController
  -> 创建并持有 UGFInputManager
  -> 在 SetupInputComponent 中传入 UEnhancedInputComponent
  -> 在 BeginPlay / OnPossess / OnRep_Pawn 中刷新输入上下文
  -> 提供 RegisterInputBindings() 虚函数

AGPPlayerController
  -> 重写 RegisterInputBindings()
  -> 注册具体玩法 InputAction 和回调

UGFInputManager
  -> 管理 InputMappingContext
  -> 绑定 InputAction
  -> 保存绑定句柄
  -> 支持按组启用、禁用、清理绑定
```

## 组件职责

### `AGFPlayerController`

- 在构造或 `BeginPlay` 中创建 `UGFInputManager`。
- 在 `SetupInputComponent` 中检查 `InputComponent` 是否为 `UEnhancedInputComponent`。
- 将 `UEnhancedInputComponent` 传给 `UGFInputManager`。
- 调用 `RegisterInputBindings()`，让子类注册具体输入。
- 在 `BeginPlay`、`OnPossess`、`OnRep_Pawn` 中通知 `InputManager` 刷新本地玩家输入子系统。
- 提供 Blueprint 或 C++ 可访问的 `GetInputManager()`。

### `UGFInputManager`

- 缓存拥有者 `AGFPlayerController`。
- 缓存当前 `UEnhancedInputComponent`。
- 获取并缓存 `UEnhancedInputLocalPlayerSubsystem`。
- 添加和移除 `UInputMappingContext`。
- 绑定 `UInputAction` 到回调。
- 为每个绑定保存 `FEnhancedInputActionEventBinding` 的句柄或可移除标识。
- 使用输入分组管理绑定，例如：
  - `Movement`
  - `Camera`
  - `Combat`
  - `UI`
  - `System`
- 提供接口：
  - `Initialize(AGFPlayerController*)`
  - `SetInputComponent(UEnhancedInputComponent*)`
  - `RefreshLocalPlayerSubsystem()`
  - `AddMappingContext(UInputMappingContext*, int32 Priority)`
  - `RemoveMappingContext(UInputMappingContext*)`
  - `ClearMappingContexts()`
  - `DisableInputGroup(FName GroupName)`
  - `EnableInputGroup(FName GroupName)`
  - `ClearBindings()`

### `AGPPlayerController`

- 重写 `RegisterInputBindings()`。
- 在其中调用 `InputManager` 注册项目具体输入。
- 具体 `InputAction` 资产由 `AGPPlayerController` 或其蓝图子类配置。
- 回调函数仍由 `AGPPlayerController`、Pawn 或 Character 实现，`InputManager` 只负责连接关系。

## 输入分组策略

输入绑定应带有分组名称。运行时禁用某个分组时，`InputManager` 移除该分组下的绑定；恢复时重新绑定。

推荐分组：

- `Movement`：移动、跳跃、冲刺等角色移动输入。
- `Camera`：鼠标或手柄镜头输入。
- `Combat`：攻击、防御、技能、锁定目标。
- `UI`：菜单、确认、取消、导航。
- `System`：暂停、调试、截图等不随玩法状态关闭的输入。

对于打开背包、对话、过场等场景，调用方可以选择关闭一个或多个分组。例如打开对话时关闭 `Movement` 和 `Combat`，但保留 `Camera` 或 `UI`。

## 初始化流程

1. `AGFPlayerController::BeginPlay()` 创建或初始化 `UGFInputManager`。
2. `UGFInputManager::Initialize()` 缓存 PlayerController，并尝试获取 `LocalPlayerSubsystem`。
3. `AGFPlayerController::SetupInputComponent()` 调用父类后，将 `InputComponent` 转为 `UEnhancedInputComponent`。
4. 转换成功后调用 `InputManager->SetInputComponent()`。
5. `AGFPlayerController` 调用虚函数 `RegisterInputBindings()`。
6. `AGPPlayerController::RegisterInputBindings()` 注册具体 `InputAction`、触发事件、回调和分组。
7. `OnPossess` 或 `OnRep_Pawn` 触发时，`InputManager` 刷新拥有 Pawn 相关绑定或上下文。

## 运行时禁用流程

当游戏状态需要禁用某组输入时，调用：

```text
InputManager->DisableInputGroup("Combat")
```

`InputManager` 查找该分组下的所有绑定，将它们从 `UEnhancedInputComponent` 中移除，并记录分组处于禁用状态。

恢复时调用：

```text
InputManager->EnableInputGroup("Combat")
```

`InputManager` 根据之前保存的绑定描述重新绑定对应 `InputAction`。

## 错误处理

- 如果没有本地玩家，不添加 MappingContext，并输出日志。
- 如果 `InputComponent` 不是 `UEnhancedInputComponent`，跳过绑定并输出日志。
- 如果 `InputAction` 或 `InputMappingContext` 为空，忽略该项并输出警告。
- 重复添加同一个 MappingContext 时应避免重复添加。
- 重复绑定同一个分组和 Action 时应先清理旧绑定，避免回调被触发多次。

## 测试与验证

手动验证：

- 启动地图后确认基础输入绑定成功。
- 打开一个测试状态时调用 `DisableInputGroup("Movement")`，确认移动输入停止响应。
- 调用 `EnableInputGroup("Movement")` 后确认移动恢复。
- 切换 Pawn 或重新 Possess 后确认绑定仍然有效。
- 在非本地 PlayerController 上确认不会错误添加本地 MappingContext。

自动化或日志验证：

- 为 `InputManager` 添加日志分类，记录初始化、绑定、解绑、MappingContext 添加和移除。
- 后续可以增加轻量自动化测试，验证重复绑定不会产生重复回调。

## 实施边界

第一阶段只实现框架能力：

- `UGFInputManager`
- `AGFPlayerController` 初始化入口
- `AGPPlayerController` 的注册扩展点
- 基础分组启用和禁用接口

具体玩法输入资产和蓝图配置可以在第二阶段接入。

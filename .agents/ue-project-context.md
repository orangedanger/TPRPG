# UE Project Context

*Last updated: 2026-06-29*

## Engine & Project Overview
**Engine version:** UE 5.7, local engine source/build at `D:\soft\UE_5.7\Engine`  
**Project name:** `TPRPG`  
**Project path:** `D:\Project\UE_Project\TPRPG`  
**Description:** Third-person RPG combat demo project for UE C++ social recruitment preparation.  
**Project type:** Game  
**Genre / domain:** Third-person action RPG / small ARPG combat training room  
**Target platforms:**
- Windows desktop first

## Module Structure
**Primary game module:** `GPGameplay`

| Module | Type | Notes |
| --- | --- | --- |
| `GPGameplay` | Runtime | Primary game module for gameplay code |
| `UnrealMCP` | Editor plugin | Local editor MCP helper plugin under `Plugins/UnrealMCP` |

**Key dependencies per module:**
- **GPGameplay**:
  - PublicDeps: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`
  - PrivateDeps: none currently
- **UnrealMCP**:
  - Editor plugin; depends on `EditorScriptingUtilities`

## Plugin Dependencies
**Engine plugins enabled:**
- `GameplayAbilities` - enabled in `.uproject`, not part of first-week Demo scope.
- `ModelingToolsEditorMode` - editor-only modeling tools.
- `EnhancedInput` - used by module dependency and project input defaults.

**Project plugins:**
- `UnrealMCP` - editor-only Model Context Protocol helper plugin.

**Notable configuration:**
- Enhanced Input defaults are set in `Config/DefaultInput.ini`:
  - `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`
  - `DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`

## Coding Conventions
**Naming prefixes:** Standard UE prefixes (`F`, `U`, `A`, `E`, `I`)  
**Header style:** `#pragma once`  
**Header organization:** Current project uses a flat primary module layout under `Source/GPGameplay`; future gameplay classes may be grouped by feature folders if needed.  
**Pointer rules:**
- Use `UPROPERTY() TObjectPtr<T>` for stored UObject/component references where GC visibility matters.
- Use raw UObject pointers for local variables and function parameters when lifetime is externally owned.
- Do not manage `UObject` lifetime with `std::shared_ptr` / `TSharedPtr`.

## Subsystems in Use
**Gameplay framework:**
- GameMode: not customized yet.
- GameState: not customized yet.
- PlayerController: not customized yet.
- Pawn / Character: not customized yet.

**Planned first Demo classes:**
- `ACombatDemoCharacter` - player character, movement/input owner.
- `ATrainingEnemy` - first wood-dummy enemy target.
- `UAttributeComponent` - health and death state.
- `UCombatComponent` - attack request, `SphereTrace`, damage application.
- `FSkillRow` - `DataTable` row for one `BasicAttack` skill.

**Subsystems:**
- None established yet.

**GAS usage:**
- `GameplayAbilities` plugin is enabled, but first Demo scope will not use full GAS.

## Build Configuration
**Build targets:**
- `TPRPGTarget` - Game
- `TPRPGEditorTarget` - Editor

**Build settings:**
- `DefaultBuildSettings = BuildSettingsVersion.V6`
- `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7`

**Custom macros / build flags:**
- None currently.

**Third-party libraries:**
- None currently.

**Engine modifications:**
- None documented for project code.

## Current Demo Scope
First playable slice:
- Single-player only.
- Wood-dummy enemy only; no AI behavior tree or perception.
- Attack uses `SphereTrace`.
- Feedback uses `UE_LOG`, `DrawDebug`, and/or DebugString.
- No formal UMG in first week.
- One `DataTable` row only: `BasicAttack`.
- Death feedback can be hide, destroy, or death log.

## Notes For Future Tasks
- Keep the first implementation small and observable.
- Do not start with networking, full GAS, complex UI, or AI.
- Preserve clear component responsibility boundaries:
  - `AttributeComponent` owns health/death state.
  - `CombatComponent` owns hit detection and damage requests.
  - Skill data is read from `DataTable`, but the first version only needs one row.

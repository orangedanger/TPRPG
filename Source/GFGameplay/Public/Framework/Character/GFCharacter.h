// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GFCharacter.generated.h"

/**
 * 基础角色类，为当前第三人称角色提供共享移动配置和相机组件。
 * 该类不缓存 Controller 或处理输入，具体输入响应由玩法层派生角色实现。
 */
UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGFCharacter();

	/** 返回第三人称相机臂，供蓝图或派生类读取相机挂载状态。 */
	UFUNCTION(BlueprintPure, Category = "GP|Character")
	USpringArmComponent* GetCameraBoom() const;

	/** 返回跟随相机，供蓝图或派生类读取当前相机组件。 */
	UFUNCTION(BlueprintPure, Category = "GP|Character")
	UCameraComponent* GetFollowCamera() const;

protected:
	/** 第三人称相机臂，负责跟随角色并响应控制器旋转。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 第三人称跟随相机，挂在 CameraBoom 上用于本地视角表现。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
};

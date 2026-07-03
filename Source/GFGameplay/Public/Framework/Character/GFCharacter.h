// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GFCharacter.generated.h"

class AGFPlayerController;

/**
 * 基础角色类，集中提供所有玩法角色共享的相机组件和当前控制器缓存。
 * 派生角色可通过缓存的 PlayerController 订阅输入委托或读取控制旋转。
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
	virtual void BeginPlay() override;
	
	/** 第三人称相机臂，负责跟随角色并响应控制器旋转。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 第三人称跟随相机，挂在 CameraBoom 上用于本地视角表现。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	/** 当前控制该角色的 GF PlayerController，派生类用它访问输入委托和控制旋转。 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AGFPlayerController> PlayerController;
};

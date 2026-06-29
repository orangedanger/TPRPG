// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GFCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class GFGAMEPLAY_API AGFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGFCharacter();

	UFUNCTION(BlueprintPure, Category = "GP|Character")
	USpringArmComponent* GetCameraBoom() const;

	UFUNCTION(BlueprintPure, Category = "GP|Character")
	UCameraComponent* GetFollowCamera() const;
protected:
	virtual void BeginPlay() override;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GP|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
};

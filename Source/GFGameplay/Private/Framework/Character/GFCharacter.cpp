// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Character/GFCharacter.h"

#include "Camera/CameraComponent.h"
#include "Framework/PlayerController/GFPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AGFCharacter::AGFCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	// GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	// GetCharacterMovement()->JumpZVelocity = 500.0f;
	// GetCharacterMovement()->AirControl = 0.35f;
	// GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	// GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	// GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AGFCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 当前 Demo 在 BeginPlay 缓存一次控制器，派生类后续通过该指针访问输入委托。
	PlayerController = Cast<AGFPlayerController>(GetController());
}

USpringArmComponent* AGFCharacter::GetCameraBoom() const
{
	return CameraBoom;
}

UCameraComponent* AGFCharacter::GetFollowCamera() const
{
	return FollowCamera;
}

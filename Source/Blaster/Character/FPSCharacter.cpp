// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
AFPSCharacter::AFPSCharacter()
{
	FollowCamera->SetupAttachment(GetMesh(), FName("head"));
	//组件销毁用DestroyComponent函数
	CameraBoom->DestroyComponent();
}

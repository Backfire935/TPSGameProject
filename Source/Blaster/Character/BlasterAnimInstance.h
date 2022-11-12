// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;

	FORCEINLINE void SetbUseFABRIK(bool bUse)  { bUseFABRIK = bUse; }
private:
	UPROPERTY(BlueprintReadOnly, category = Character, meta = (AllowPrivateAccess = "ture"))
	class ABlasterCharacter* BlasterCharacter;
	
	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bIsInAir;//是否在空中

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bIsAccelerating;//是否在加速

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bWeaponEquipped;//是否装备武器

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bIsCrouched;//是否下蹲

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bAiming;//是否瞄准

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float YawOffset;//角色倾向左右的偏移量

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float Lean;//角色动作向前向后的量

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float AO_Pitch;

	
	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
	FTransform LeftHandTransform;
	 
	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
	FRotator RightHandRotation; 

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bElimed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bUseFABRIK	;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bTransformRightHand;
};

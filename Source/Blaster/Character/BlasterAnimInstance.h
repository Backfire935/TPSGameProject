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
		bool bIsInAir;//�Ƿ��ڿ���

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bIsAccelerating;//�Ƿ��ڼ���

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bWeaponEquipped;//�Ƿ�װ������

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bIsCrouched;//�Ƿ��¶�

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		bool bAiming;//�Ƿ���׼

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float YawOffset;//��ɫ�������ҵ�ƫ����

	UPROPERTY(BlueprintReadOnly, category = Movement, meta = (AllowPrivateAccess = "ture"))
		float Lean;//��ɫ������ǰ������

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

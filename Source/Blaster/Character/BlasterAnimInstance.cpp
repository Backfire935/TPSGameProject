// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include"Blaster/BlasterTypes/CombatState.h"
void UBlasterAnimInstance::NativeInitializeAnimation()
{	
	Super::NativeInitializeAnimation();

	BlasterCharacter =Cast<ABlasterCharacter>(TryGetPawnOwner());    //TryGetPawnOwner��ȡPawn�࣬Cast��������ת��
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)	//����ָ��!!!
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());    //TryGetPawnOwner��ȡPawn�࣬Cast��������ת��
	}

	if (BlasterCharacter == nullptr)	return;//����ת��ʧ�ܣ�return	
	
	FVector  Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size(); //��ȡ��ά������ģ��������ZΪ0
	
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();		//��ɫ�Ƿ��ڿ���

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //��ȡ��ɫ��ǰ���ٶȣ�ֻҪ��0������ڼ���

	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();//��ȡ�Ƿ�װ������

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();//��ȡװ��������

	bIsCrouched = BlasterCharacter->bIsCrouched;//��ȡ�Ƿ��¶�

	bAiming = BlasterCharacter->IsAiming();//��ȡ�Ƿ�����׼

	TurningInPlace = BlasterCharacter->GetTurningInPlace();//��ȡ��ɫ����

	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();//��ȡģ���ɫ�Ƿ�Ӧ����ת����

	bElimed = BlasterCharacter->IsElimed();//���ý�ɫ��ʼ�Ƿ���̭

	FRotator AimRotation =  BlasterCharacter->GetBaseAimRotation();//��ȡ����ת��

	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//ÿһ֡����MovementRotation-AimRotation��ĽǶȲ�

	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 3.f);
	YawOffset = DeltaRotation.Yaw;//����ƫ����
	 
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 3.f);//��ǰ��Ŀ�꣬ƽ��ʱ������ֵ
	Lean = FMath::Clamp(Interp, -90.f, 90.f);//����ƫ�Ʊ߽�

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);//��ȡ���

		FVector OutPosition;
		FRotator OutRotation;

		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));//�������ת ���ֵ�λ�õ����е�λ��
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation , DeltaTime, 30.f);
		}
		
		bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;//ֻҪ������ͨ״̬������IK����
		if(BlasterCharacter->IsLocallyControlled()  && BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade)
		{
			bUseFABRIK = !BlasterCharacter->IsLocallyReloading();//ֻҪ���ز��Ǵ��ڻ�������������
		}
		bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGamePlay();////ֻҪ������ͨ״̬������IK����,����AimOffsets���ҵ���ֹ�����ָ������ʱҲ����
		bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGamePlay();//����״̬�½��������ƶ����ҵ���ֹ�����ָ������ʱҲ����
	/*	FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(),BlasterCharacter->GetHitTarget(), FColor::Orange);���߼����ײ*/
	}


}

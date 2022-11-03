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

	BlasterCharacter =Cast<ABlasterCharacter>(TryGetPawnOwner());    //TryGetPawnOwner获取Pawn类，Cast进行类型转换
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)	//检查空指针!!!
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());    //TryGetPawnOwner获取Pawn类，Cast进行类型转换
	}

	if (BlasterCharacter == nullptr)	return;//类型转换失败，return	
	
	FVector  Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size(); //获取三维向量的模长，其中Z为0
	
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();		//角色是否在空中

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false; //获取角色当前加速度，只要比0大就是在加速

	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();//获取是否装备武器

	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();//获取装备的武器

	bIsCrouched = BlasterCharacter->bIsCrouched;//获取是否下蹲

	bAiming = BlasterCharacter->IsAiming();//获取是否在瞄准

	TurningInPlace = BlasterCharacter->GetTurningInPlace();//获取角色朝向

	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();//获取模拟角色是否应该旋转骨骼

	bElimed = BlasterCharacter->IsElimed();//设置角色初始是否被淘汰

	FRotator AimRotation =  BlasterCharacter->GetBaseAimRotation();//获取鼠标的转动

	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);//每一帧计算MovementRotation-AimRotation后的角度差

	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 3.f);
	YawOffset = DeltaRotation.Yaw;//左右偏移量
	 
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 3.f);//当前，目标，平滑时长，插值
	Lean = FMath::Clamp(Interp, -90.f, 90.f);//设置偏移边界

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);//获取插槽

		FVector OutPosition;
		FRotator OutRotation;

		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));//看向的旋转 右手的位置到击中的位置
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation , DeltaTime, 30.f);
		}
		
		bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;//只要不是普通状态都禁用IK动画
		if(BlasterCharacter->IsLocallyControlled()  && BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade)
		{
			bUseFABRIK = !BlasterCharacter->IsLocallyReloading();//只要本地不是处于换弹动画都开启
		}
		bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGamePlay();////只要不是普通状态都禁用IK动画,禁用AimOffsets而且当禁止输入的指令设置时也禁用
		bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGamePlay();//换弹状态下禁用右手移动而且当禁止输入的指令设置时也禁用
	/*	FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(),BlasterCharacter->GetHitTarget(), FColor::Orange);射线检测碰撞*/
	}


}

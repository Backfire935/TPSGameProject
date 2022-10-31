// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include <string>

#include "TimerManager.h"
#include "Blaster/Character/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ChargeRampUp(DeltaTime);
}




void UBuffComponent::JumpUp(float LastJumpVelocity,float JumpVelocity, float JumpupTime)
{
	if (Character == nullptr) return;

	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	InitialJumpZVelocity = LastJumpVelocity;
	Character->GetWorldTimerManager().SetTimer(
		SpeedUpTimer,
		this,
		&UBuffComponent::ResetJump,
		JumpupTime
	);
	MulticastJumpBuff(JumpVelocity);
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;
	}
	MulticastJumpBuff(InitialJumpZVelocity);
}



void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}


void UBuffComponent::SpeedUp(float DefaultWalkSpeed,float DefaultCrouchSpeed, float AcclerateSpeedWalk, float AcclerateCrouch, float SpeedTime)
{
	if (Character == nullptr) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = AcclerateSpeedWalk;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = AcclerateCrouch;

	LastWalkSpeed = DefaultWalkSpeed;
	LastCrouchSpeed = DefaultCrouchSpeed;
	Character->GetWorldTimerManager().SetTimer(
		SpeedUpTimer,
		this,
		&UBuffComponent::SpeedDown,
		SpeedTime
	);
	MulticastSpeedBuff(AcclerateSpeedWalk, AcclerateCrouch);//�ಥһ���������ͻ���Ҳ֪���Լ����ٶ�������
}

void UBuffComponent::SpeedDown() 
{
	Character->GetCharacterMovement()->MaxWalkSpeed = LastWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = LastCrouchSpeed;
	MulticastSpeedBuff(LastWalkSpeed, LastCrouchSpeed);//ʱ�䵽�ˣ��ಥһ�½�����

}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}


void UBuffComponent::Heal(float HealAmount, float HealingTime)//�������ֻ���������ò����ģ������������Ƶ���tickcomponentt����healrampup
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimed()) return;//Ҫ�ǽ�ɫ�����ڻ����Ѿ�����̭�˾Ͳ�������

	const float HealThisFrame = HealingRate * DeltaTime;//ÿһ֡�Ļ�Ѫ��
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));//����Ѫ��
	Character->UpdateHUDHealth();

	HealedAmount += HealThisFrame * AmountToHeal;//��Ѫ���ļ���
	if (Character->IsElimed() || Character->GetHealth() == Character->GetMaxHealth() || HealedAmount == AmountToHeal) //������ƹ�������ұ���̭�˻���Ѫ���ָ��������ֵ����Ѫ���Ѿ��ָ�������������Ч�����ǻ�û������ֹͣ����
	{
		HealedAmount = 0;
		AmountToHeal = 0;
		bHealing = false;
	}
}

void UBuffComponent::RechargeShiled(float RechargeAmount, float RechargingTime ,bool bSlowly)
{
	bRecharge = true;
	bSlowlyCharged = bSlowly;
	ChargingShiledRate = RechargeAmount / RechargingTime;
	AmountToRechargeShiled += RechargeAmount;
	FString Message2 = FString::SanitizeFloat(AmountToRechargeShiled);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			60,
			FColor::Red,
			Message2
		);
	}
}

void UBuffComponent::ChargeRampUp(float DeltaTime)
{
	if (!bRecharge || Character == nullptr || Character->IsElimed()) return;//Ҫ�ǽ�ɫ�����ڻ����Ѿ�����̭�˾Ͳ�������

	if(bSlowlyCharged)//�Ƿ���˲��ָ�����
	{
		const float RechargeThisFrame = ChargingShiledRate * DeltaTime;//ÿһ֡�Ļ�Ѫ��
		Character->SetShield(FMath::Clamp(Character->GetShield() + RechargeThisFrame, 0.f, Character->GetMaxShield()));//���û��ܵ���ֵ
		Character->UpdateHUDShield();//����һ�λ���HUD
		RechargedAmount+= RechargeThisFrame * AmountToRechargeShiled;//�Ѿ��ָ��Ļ��ܵļ���

		//FString Message1 = FString::SanitizeFloat(RechargedAmount);

		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(
		//		-1,
		//		5,
		//		FColor::Yellow,
		//		Message1
		//	);
		//}
	
		if (Character->IsElimed() || Character->GetShield() == Character->GetMaxShield() || RechargedAmount == AmountToRechargeShiled) //������ƹ�������ұ���̭�˻���Ѫ���ָ��������ֵ����Ѫ���Ѿ��ָ�������������Ч�����ǻ�û������ֹͣ����
		{
			RechargedAmount = 0;//���ܼ�������
			AmountToRechargeShiled = 0;
			bRecharge = false;//���ܽ���
			return;
		}
	}
	else//˲��ָ����ܵĻ�
	{
		Character->SetShield(FMath::Clamp(Character->GetShield() + AmountToRechargeShiled, 0.f, Character->GetMaxShield()));//���û���
		Character->UpdateHUDShield();
		AmountToRechargeShiled = 0;
		bRecharge = false;//���ܽ���
		return;
	}



}


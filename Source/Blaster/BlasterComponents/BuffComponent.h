// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UBuffComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);

	void RechargeShiled(float RechargeAmount, float RechargingTime , bool bSlowly);

	void SpeedUp (float DefaultWalkSpeed,float DefaultCrouchSpeed,float AcclerateSpeed, float AcclerateCrouch,float SpeedTime) ;

	void SpeedDown();

	void JumpUp(float LastJumpVelocity,float JumpVelocity, float JumpupTime);
protected:

	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

	void ChargeRampUp(float DeltaTime);

private:
	UPROPERTY()
		class ABlasterCharacter* Character;

	//��������buff
	bool bHealing = false;//�Ƿ�����Ѫ״̬

	float HealingRate = 0;//��Ѫ����

	float AmountToHeal = 0;//��Ѫ����

	float HealedAmount = 0;//���ڴ洢�Ѿ��ָ���Ѫ��

	//���ڻ���buff
	bool bRecharge = false;//�Ƿ����ָ�����״̬

	float ChargingShiledRate = 0;//�ָ���������

	float AmountToRechargeShiled = 0;//�ָ���������

	float RechargedAmount = 0;//���ڴ洢�Ѿ��ָ��Ļ���

	bool bSlowlyCharged = false;//�ǻ����ָ����ܻ���һ�γ���

	//���ڼ���buff
	float LastWalkSpeed = 0.f;
	float LastCrouchSpeed = 0.f;

	FTimerHandle SpeedUpTimer;//���ٵļ�ʱ��

	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	//������Ծbuff
	FTimerHandle JumpBuff;
	
	void ResetJump();

	float InitialJumpZVelocity;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastJumpBuff(float JumpVelocity);
public:	


		
};


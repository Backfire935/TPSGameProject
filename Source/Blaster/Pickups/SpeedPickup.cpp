// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/BuffComponent.h"

//ʹ��ǰ������ͼ������Ĭ��ֵ�Ƿ����ã�C++�е��ٶ�Ĭ��ֵ��0��С��һ�����˶�����
ASpeedPickup::ASpeedPickup()
{
	bReplicates = true;
}

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//if (BlasterCharacter->GetHealth() == BlasterCharacter->GetMaxHealth()) return;//��Ѫ״̬��������Ч
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		CurrentWalkSpeed = BlasterCharacter->GetCharacterMovement()->GetMaxSpeed();//��ȡ��ǰ�ٶ�
		CurrentCrouchSpeed = BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched;
		if (Buff)
		{
			Buff->SpeedUp(CurrentWalkSpeed,CurrentCrouchSpeed, AcclerateToWalk, AcclerateToCrouch,SpeedUpTime);//���accleratespeed����������ͼ���úã���Ȼ������Ĭ���ٶ���0
		}
	}
	Destroy();
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		if(!BlasterCharacter->IsWeaponEquipped())//���ûװ�������������·���
		{
			return;
		}
		if(BlasterCharacter->GetEquippedWeapon()->GetWeaponType() != WeaponType )//���ʰȡ�ĵ�ҩ�����ϵ��������಻ͬ�������·���
		{
			return;
		} 
		UCombatComponent* Combat = BlasterCharacter->GetCombat();
		if(Combat)
		{
			Combat->PickupAmmp(WeaponType,AmmoAmount);//������Ҫ��ӵ�������ҩ���ͺ͵�ҩ��Ŀ
		}
		
	}
	Destroy();
}

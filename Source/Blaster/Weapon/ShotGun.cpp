// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include"Engine/SkeletalMeshSocket.h"
#include"Blaster/Character/BlasterCharacter.h"
#include"Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
void AShotGun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)//һ��ֱ�������������
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();//���������ʼ��
		uint32 Hits = 0;//ÿ��������һ�Σ�����һ�α����д���

		TMap<ABlasterCharacter*, uint32> HitMap;//�������еĶ��󶼼ӽ�ȥ

		for(uint32 i = 0; i < NumberOfPellets; i++)
		{
			//FVector End = TraceWithScatter(HitTarget);//����ɢ������߼��
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//��ȡ���е�Ŀ��
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if(HitMap.Contains(BlasterCharacter))//�Ѿ����й������Hits��+1
				{
					HitMap[BlasterCharacter]++;
				}
				else//��ǰû���й�����˾���ӽ�ȥ������HitsΪ1
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}

			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-0.5F, 0.5F)
				);
			}
		}

		for(auto HitPair : HitMap)//��ʼ����ÿ�����н�ɫ�����˺�
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,//Map�еĽ�ɫ
					HitPair.Value * Damage,//Map�н�ɫ�ܵ����˺�����*�����˺�ֵ
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);

			}
		}
	}

}


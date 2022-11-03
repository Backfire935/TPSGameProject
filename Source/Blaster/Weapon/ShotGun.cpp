// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include"Engine/SkeletalMeshSocket.h"
#include"Blaster/Character/BlasterCharacter.h"
#include"Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)//һ��ֱ�������������
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();//���������ʼ��

		//��Ӧ���еĽ�ɫ�ͱ����еĴ���
		TMap<ABlasterCharacter*, uint32> HitMap;//�������еĶ��󶼼ӽ�ȥ

		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//��ȡ���е�Ŀ��
			if (BlasterCharacter )//�ڱ���ִ�У������Ȩ���Ժ�����˺�����ҿ�����
			{
				if (HitMap.Contains(BlasterCharacter))//�Ѿ����й������Hits��+1
				{
					HitMap[BlasterCharacter]++;
				}
				else//��ǰû���й�����˾���ӽ�ȥ������HitsΪ1
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}

				if (ImpactParticles)//���ɴ��������Ч��
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				if (HitSound)//����������Ч
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

		}
		for (auto HitPair : HitMap)//��ʼ����ÿ����ɫ�����е����˺�
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

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)//��ȡ�����ӵ�Ⱥ�����
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return;//һ��ֱ�������������

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();//���������ʼ��

	const FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//һ����������ʼ�㵽������Ŀ�������
	const FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//����������յ���е�����


	for(uint32 i =0; i < NumberOfPellets; i++)//����ѭ����СΪһ�η����ӵ�������
	{
		const FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//�������λ����*������� 
		const FVector EndLoc = SphereCenter + RandVec;//���ĵ����ܵ������ɢ����
		 FVector ToEndLoc = EndLoc - TraceStart;//�������߶�
		 FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//������������

		HitTargets.Add(EndEnd);
	}
}



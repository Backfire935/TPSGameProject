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

	if (MuzzleFlashSocket)//一个直线武器攻击检测
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();//开火检测的起始点
		uint32 Hits = 0;//每当被命中一次，增加一次被命中次数

		TMap<ABlasterCharacter*, uint32> HitMap;//将被命中的对象都加进去

		for(uint32 i = 0; i < NumberOfPellets; i++)
		{
			//FVector End = TraceWithScatter(HitTarget);//喷子散射的射线检测
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//获取击中的目标
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if(HitMap.Contains(BlasterCharacter))//已经击中过这个人Hits就+1
				{
					HitMap[BlasterCharacter]++;
				}
				else//以前没击中过这个人就添加进去并且置Hits为1
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

		for(auto HitPair : HitMap)//开始计算每个命中角色的总伤害
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,//Map中的角色
					HitPair.Value * Damage,//Map中角色受到的伤害次数*单词伤害值
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);

			}
		}
	}

}


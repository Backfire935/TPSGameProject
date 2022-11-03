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

	if (MuzzleFlashSocket)//一个直线武器攻击检测
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();//开火检测的起始点

		//对应命中的角色和被命中的次数
		TMap<ABlasterCharacter*, uint32> HitMap;//将被命中的对象都加进去

		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//获取击中的目标
			if (BlasterCharacter )//在本地执行，不检查权威性和造成伤害的玩家控制器
			{
				if (HitMap.Contains(BlasterCharacter))//已经击中过这个人Hits就+1
				{
					HitMap[BlasterCharacter]++;
				}
				else//以前没击中过这个人就添加进去并且置Hits为1
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}

				if (ImpactParticles)//生成打击的贴花效果
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				if (HitSound)//生成命中音效
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
		for (auto HitPair : HitMap)//开始计算每个角色被命中的总伤害
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

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)//获取喷子子弹群的落点
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return;//一个直线武器攻击检测

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();//开火检测的起始点

	const FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//一个从射线起始点到被击中目标的向量
	const FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//到喷子射程终点的中点向量


	for(uint32 i =0; i < NumberOfPellets; i++)//设置循环大小为一次发射子弹的数量
	{
		const FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//随机方向单位向量*随机长度 
		const FVector EndLoc = SphereCenter + RandVec;//中心到四周的随机扩散向量
		 FVector ToEndLoc = EndLoc - TraceStart;//两点间的线段
		 FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//单个射线向量

		HitTargets.Add(EndEnd);
	}
}



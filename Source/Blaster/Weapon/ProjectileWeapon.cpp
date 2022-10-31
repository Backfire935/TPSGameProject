// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);//调用父类的开火动画函数

	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket * MuzzleFlashSocket = 	GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));//获取武器枪口的插槽
	if (MuzzleFlashSocket)//枪口插槽的位置
	{
		FTransform SocketTransform = 	MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//获取当前手上的武器的枪口插槽
		
		//从命中目标-枪口插槽的向量
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			UWorld* World = GetWorld();
			SpawnParams.Owner = GetOwner();//设置发射物的拥有者
			SpawnParams.Instigator = InstigatorPawn;//发起对象
			if (World)
			{
				World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
			}
		}
	}


}

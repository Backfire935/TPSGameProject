// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);//���ø���Ŀ��𶯻�����

	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket * MuzzleFlashSocket = 	GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));//��ȡ����ǹ�ڵĲ��
	if (MuzzleFlashSocket)//ǹ�ڲ�۵�λ��
	{
		FTransform SocketTransform = 	MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//��ȡ��ǰ���ϵ�������ǹ�ڲ��
		
		//������Ŀ��-ǹ�ڲ�۵�����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			UWorld* World = GetWorld();
			SpawnParams.Owner = GetOwner();//���÷������ӵ����
			SpawnParams.Instigator = InstigatorPawn;//�������
			if (World)
			{
				World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
			}
		}
	}


}

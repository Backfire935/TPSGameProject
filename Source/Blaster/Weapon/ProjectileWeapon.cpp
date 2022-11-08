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
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && MuzzleFlashSocket)//ǹ�ڲ�۵�λ��
	{
		FTransform SocketTransform = 	MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//��ȡ��ǰ���ϵ�������ǹ�ڲ��
		
		//������Ŀ��-ǹ�ڲ�۵�����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();//���÷������ӵ����
		SpawnParams.Instigator = InstigatorPawn;//�������

		AProjectile* SpawnedProjectile = nullptr;


		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority())//Serverʹ��ssr�������
			{
				if(InstigatorPawn->IsLocallyControlled())//���������ʹ�ÿɸ��Ƶ�����������
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
					SpawnedProjectile->bUseServerSideRewind = false;//�����û��Ҫ��SSR
					SpawnedProjectile->Damage = Damage;//��ҩ���˺� = �������˺�
				}
				else//������ϵķǱ������Ƶ�ģ�������,���ɲ��������縴�Ƶ�������(�ͻ���RPC֮������ķ����rep_notify�Ѿ�������һ����)����ʹ��SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
					SpawnedProjectile->bUseServerSideRewind = false;//�����û��Ҫ��SSR

				}
			}
			else//�ͻ���ʹ��ssr�������
			{
				if (InstigatorPawn->IsLocallyControlled())//�ͻ��˱��ؿ��Ƶ�����£����ɷǸ��Ƶ������ʹ��ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
					SpawnedProjectile->bUseServerSideRewind = true;//�ͻ��˵��ӵ����ʱ�������ssr��
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;//��ҩ���˺� = �������˺�

				}
				else//�ͻ��˷Ǳ��ؿ��ƣ����ɲ��������縴�Ƶ��������ʹ��ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
					SpawnedProjectile->bUseServerSideRewind = false;//�����û��Ҫ��SSR
				}
			}
		}
		else//������ʹ��ssr�����
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//����ĵ�ҩ�࣬����Ĳ�۵�λ�ã�Ŀ�����ת�������������һϵ����Ϣ
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;//��ҩ���˺� = �������˺�
			}
		}
	}

}

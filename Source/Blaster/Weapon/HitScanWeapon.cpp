// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include"Engine/SkeletalMeshSocket.h"
#include"Blaster/Character/BlasterCharacter.h"
#include"Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include  "WeaponTypes.h"
#include"Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)//һ��ֱ�������������
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();//���������ʼ��

		FHitResult FireHit;

		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//��ȡ���е�Ŀ��
		if (BlasterCharacter && InstigatorController)
		{
			if(HasAuthority() && !bUseServerSideRewind)//��ʹ�÷��������ݾ�ֱ�����˺�,Ŀǰ����˿���������ܾ��޷��Է��������˺�
			{
				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			 if(!HasAuthority() && bUseServerSideRewind)//�Ƿ�ʹ���˷���������,�ͻ��˲������ѡ��
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if(BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent())
				{
					BlasterOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime,
						this
					);
				}
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
				FireHit.ImpactPoint
			);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart,const FVector & HitTarget, FHitResult& OutHit)
{

	UWorld* World = GetWorld();
	if(World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25;//1.25��Ŀ��㵽�˵Ĺ�������

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		DrawDebugSphere(
			GetWorld(),
			BeamEnd,
			16.f,
			12,
			FColor::Orange,
			true
		);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

	}
}

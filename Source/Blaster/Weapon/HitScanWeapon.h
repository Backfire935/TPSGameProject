// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	//���߽�����λ�÷����ӵ�

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
		float DistanceToSphere = 800.f;//���ӵ���Ч��������

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;//��ɢ��Χ

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;//�Ƿ������ӵ�ɢ��

protected:
	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		float Damage = 10.f;


	FVector TraceWithScatter(const FVector& TraceStart, const FVector& HitTarget);//����ɢ������߼��

	void WeaponTraceHit(const FVector & TraceStart, const FVector& HitTarget, FHitResult& OutHit);//�������и���
};

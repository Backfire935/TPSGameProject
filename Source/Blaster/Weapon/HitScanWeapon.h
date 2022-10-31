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

	//射线结束的位置分裂子弹

	UPROPERTY(EditAnywhere,Category="Weapon Scatter")
		float DistanceToSphere = 800.f;//喷子的有效攻击距离

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;//扩散范围

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;//是否允许子弹散射

protected:
	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		float Damage = 10.f;


	FVector TraceWithScatter(const FVector& TraceStart, const FVector& HitTarget);//喷子散射的射线检测

	void WeaponTraceHit(const FVector & TraceStart, const FVector& HitTarget, FHitResult& OutHit);//武器击中跟踪
};

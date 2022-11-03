// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotGun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);//��ȡ�����ӵ�Ⱥ�����

	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
private:
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;//���ӵ�������
};

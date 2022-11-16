// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

	virtual void Dropped() override;

	virtual void HandleWeaponEquiped() override;//处理装备过的武器
	virtual void HandleWeaponDropped() override;//处理丢弃的武器

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;


};

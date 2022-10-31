// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{	
	GENERATED_BODY()
protected:
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

		virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USoundCue* RocketLoopInAir;//���������һֱ�ɵĺ�����

	UPROPERTY()
	UAudioComponent* RocketLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;//��������Ϸɵ�������˥������

	UPROPERTY(EditAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:


public:
	UPROPERTY(EditAnywhere)
		float InnerScope = 200.f;//���������˺���Χ����

	UPROPERTY(EditAnywhere)
		float OutScope = 500.f;//�����ļ��˷�Χ����

	AProjectileRocket();

	virtual void Destroyed() override;
};

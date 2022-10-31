// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()//�ܵ������Ч��
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void SpawnTrailSystem();//�����ӵ���β��

	void StartDestroyTimer();//����3s�ӳ����ٱ�֤�������β�������ڻ����������ײ��ʱ����������
	
	void DestroyTimerFinished();

	void ExplodeDamage();

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;	//�ӵ��˶����

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;//Ҫѡ���β��������Чϵͳ

	UPROPERTY()
		class  UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
		float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
		float DamageOuterRadius = 500.f;
private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;


	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;



public:	


};

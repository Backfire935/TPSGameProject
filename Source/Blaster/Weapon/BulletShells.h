// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShells.generated.h"

UCLASS()
class BLASTER_API ABulletShells : public AActor
{
	GENERATED_BODY()
	
public:	

	ABulletShells();


protected:

	virtual void BeginPlay() override;

	UFUNCTION()//�ܵ������Ч��
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletShellMesh; 

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;//�ӵ��ǵ�������

	UPROPERTY(EditAnywhere)
	class USoundCue* BulletSound;



public:	

	virtual void Tick(float DeltaTime) override;


};

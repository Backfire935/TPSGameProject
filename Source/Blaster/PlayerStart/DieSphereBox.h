// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DieSphereBox.generated.h"

UCLASS()
class BLASTER_API ADieSphereBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADieSphereBox();


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		class UBoxComponent* HitBoxComp;
protected:


public:	

	UFUNCTION(BlueprintCallable)//����ط������UFUNCTION����̳�ʱ�༭�������ҵ� ���򲻽�����Ӧ
		void BeginHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32  OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//UFUNCTION(BlueprintCallable)//����ط������UFUNCTION����̳�ʱ�༭�������ҵ� ���򲻽�����Ӧ
	virtual  void OnHitSphere(class ABlasterCharacter* Sphere); //д class �ñ�����ȥcpp����AMyPawn

};


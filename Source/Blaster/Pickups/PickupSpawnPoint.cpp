// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

}


void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();//�õ����п�ʰȡbuff���͵�����
	if(NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses-1);
		SpawnedPickup =  GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());//��ָ��λ���������һ����ʰȡ��
		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);//��һ���ಥί�У�����ʱ��buff��ʰȡ�󣬾͵��ð󶨵�StartSpawnPickupTimer�������¼�ʱ���ɡ��õ�UE��ί�У�������������Ҫ���UFUNCTION()����
		}
	}

}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();//����buff����,�����������
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin,SpawnPickupTimeMax);//����һ���������ʱ��
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,//Timer��ʱ�������������buff����
		SpawnTime
	);

}



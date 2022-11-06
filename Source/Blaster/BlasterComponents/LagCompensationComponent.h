// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterCharacter;
//�������з���˵�������
//�ͺ󲹳���
//����������ҵ�λ��
//�洢hitbox
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;//HitBoxλ����Ϣ

	UPROPERTY()
		FRotator Rotation;//HitBox��ת��Ϣ

	UPROPERTY()
		FVector BoxExtent;//�����С��Ϣ
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
		float time;//֡��ʱ����Ϣ

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;//HitBox���Ӧ��info���ݽṹ��ɵ�map

	UPROPERTY()
	ABlasterCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitComfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

		UPROPERTY()
		TMap<ABlasterCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend  class ABlasterCharacter;

	void ShowFramePackage(const FFramePackage & Package , const FColor& Color);

	//������Ϣ������˻��ݵĺ���
	//���Ϳ�ǹ��ʼ������е��Լ�����Ŀ�����Ϣ
	UFUNCTION()
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter * HitCharacter, 
		const FVector_NetQuantize & TraceStart, 
		const FVector_NetQuantize& HitLocation , 
		float HitTime);

	UFUNCTION()
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	//RPC���ã��ͻ��������˷����hitȷ������
	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize & TraceStart,
		const FVector_NetQuantize & HitLocation,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);


private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;//˫�������ڴ洢һ��ʱ���ڵ�֡����ˮ

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.0f;//����洢��ʱ��

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage & Package);

	UFUNCTION()
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderPackage, const FFramePackage& YoungerPackage, float HitTime);//������֮֡��Ĳ�֡��

	UFUNCTION()
	FServerSideRewindResult ConfirmHit(
		const FFramePackage & Package, 
		ABlasterCharacter *	HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize&HitLocation );//���õ��İ�����Ϣ���ݽ�ɫ��λ�ã�������������������������Ƿ����к��Ƿ�ͷ�Ĳ����ṹ��

	UFUNCTION()
	void CacheBoxPositions(ABlasterCharacter *HitCharacter, FFramePackage & OutFramePackage);//�������λ����Ϣ

	UFUNCTION()
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);//�ƶ���ɫ��λ�õ�����λ��

	UFUNCTION()
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);//����ɫ��λ���ƻ�ȥ

	UFUNCTION()
	void EnableCharacterMeshCollision( ABlasterCharacter *HitCharacter, ECollisionEnabled::Type CollisionEnabled);//���ؽ�ɫģ����ײ


	void SaveFramePackage();

	//���ӵĲ���
	UFUNCTION()
	FFramePackage GetFrameToCheck(ABlasterCharacter * HitCharacter, float HitTime);


	UFUNCTION()
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
};


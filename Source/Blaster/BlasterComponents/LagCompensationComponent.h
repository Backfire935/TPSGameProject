// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//用来进行服务端倒带的类
//滞后补偿类
//跟踪所有玩家的位置
//存储hitbox
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;//HitBox位置信息

	UPROPERTY()
		FRotator Rotation;//HitBox旋转信息

	UPROPERTY()
		FVector BoxExtent;//盒体大小信息
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
		float time;//帧包时间信息

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;//HitBox与对应的info数据结构组成的map

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend  class ABlasterCharacter;

	void ShowFramePackage(const FFramePackage & Package , const FColor& Color);

	//发送信息给服务端回溯的函数
	//发送开枪起始点和命中点以及击中目标的信息
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter * HitCharacter, 
		const FVector_NetQuantize & TraceStart, 
		const FVector_NetQuantize& HitLocation , 
		float HitTime);

	//RPC调用，客户端向服务端发起的hit确认请求
	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize & TraceStart,
		const FVector_NetQuantize & HitLocation,
		float HitTime,
		 class AWeapon * DamageCauser
	);


private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;//双链表用于存储一段时间内的帧包流水

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.0f;//打算存储的时间

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage & Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderPackage, const FFramePackage& YoungerPackage, float HitTime);//计算两帧之间的插帧包
	FServerSideRewindResult ConfirmHit(
		const FFramePackage & Package, 
		ABlasterCharacter *	HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize&HitLocation );//用拿到的包的信息回溯角色的位置，并计算射线命中情况，返回是否命中和是否爆头的布尔结构体

	void CacheBoxPositions(ABlasterCharacter *HitCharacter, FFramePackage & OutFramePackage);//缓存包的位置信息
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);//移动角色的位置到包的位置
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);//将角色的位置移回去
	void EnableCharacterMeshCollision( ABlasterCharacter *HitCharacter, ECollisionEnabled::Type CollisionEnabled);//开关角色模型碰撞
	void SaveFramePackage();
};

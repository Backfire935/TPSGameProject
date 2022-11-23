// Fill out your copyright notice in the Description page of Project Settings.


#include "DieSphereBox.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GamePlayStatics.h"
#include"Blaster/PlayerController/BlasterPlayerController.h"
// Sets default values
ADieSphereBox::ADieSphereBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HitBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxComp"));

	//StaticMesh->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
	//StaticMesh->SetupAttachment(HitBoxComp);
	HitBoxComp->OnComponentBeginOverlap.AddDynamic(this, &ADieSphereBox::BeginHit); //绑定HitBox组件到BeginHit函数 进去搜宏找参数列表
}


void ADieSphereBox::BeginHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABlasterCharacter>(OtherActor))//如果OtherActor转换ASphereBase成功的话 就执行下面的
	{
		ABlasterCharacter* Sphere = Cast<ABlasterCharacter>(OtherActor);
		OnHitSphere(Sphere);
	}

}

void ADieSphereBox::OnHitSphere(ABlasterCharacter* Sphere)
{
	ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(Sphere->Controller);
	//自己对自己造成10000伤害
	UGameplayStatics::ApplyDamage(
		Sphere,
		10000.f,//造成一万点伤害
		OwnerController,
		this,
		UDamageType::StaticClass()
	);
}

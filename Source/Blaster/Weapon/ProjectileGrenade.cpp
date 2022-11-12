// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);//将组件设置为可网络复制
	ProjectileMovementComponent->bShouldBounce = true;//允许弹跳

}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();//此处不用射线检测和onhit绑定，所以不写super

	SpawnTrailSystem();
	StartDestroyTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage(Damage);//应用伤害
	Super::Destroyed();
}

void AProjectileGrenade::ExplodeDamage(float damage)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)//应用一个球形范围伤害
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//世界上下文对象 
				damage,//基本伤害
				0.3 * damage,//最低伤害
				GetActorLocation(),//在击中的位置设为伤害的中心
				DamageInnerRadius,//满伤害距离
				DamageOuterRadius,//减伤距离
				1.f,//伤害衰减计算是指数计算，设置为1后可以得到一个按距离的线性伤害衰减
				UDamageType::StaticClass(),//伤害类型
				TArray<AActor*>(),//忽略对指定类型的物体的伤害
				this,//产生伤害的对象
				FiringController//谁开火的
			);//带距离衰减的球形范围伤害
		}
	}
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{

	if(BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}

}


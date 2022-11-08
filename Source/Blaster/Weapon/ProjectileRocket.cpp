// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "ProjectileBullet.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include"RocketMovementComponent.h"
AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);//���������Ϊ�����縴��
	RocketMovementComponent->InitialSpeed = InitialSpeed;
	RocketMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileRocket::Destroyed()
{

	Super::Destroyed();
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(URocketMovementComponent, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}

}
#endif

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);//����ӵ�ײ��ʱ�Ĵ��Ч��
	}

	SpawnTrailSystem();//����β��

	if(RocketLoopInAir && LoopingSoundAttenuation)//���ɵ����ĺ�����
	{
		RocketLoopComponent = UGameplayStatics::SpawnSoundAttached(
				RocketLoopInAir,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor == GetOwner())//����жϳ�Ҫը��Ŀ�����Լ��Ļ�
	{
		return;
	}
	ExplodeDamage();//Ӧ���˺�

	StartDestroyTimer();//����3s�ӳ����ٱ�֤�������β�������ڻ����������ײ��ʱ����������

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());//�ڱ����е�λ�÷���һ��ǹ�۵�����
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());//�ڱ����е�λ�ò���һ�������е���Ч
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);//������ģ�Ͳ��ɼ�����3s�ٱ���������ģ��ʵ��
	}

	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);//ͬʱ���û��������ײ����Ϊ������ײ
	}

	if(TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();//���ټ�������ϵͳ���Ͳ������ͻͻͻðβ��
	}

	if(RocketLoopInAir && RocketLoopComponent->IsPlaying())//���ײ��Ŀ����ڲ�������
	{
		RocketLoopComponent->Stop();//ֹͣ���ź�����
	}
//	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}




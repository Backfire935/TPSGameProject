// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include"Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "NiagaraFunctionLibrary.h"
// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//������ײͨ��
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//���ý�������ײ
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);



}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);//���ɿ���켣
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this , &AProjectile::OnHit);//����ӵ�ײ��ʱ�Ĵ��Ч��
	}

//	CollisionBox->IgnoreActorWhenMoving();//����ɫ���ƶ���ʱ�������ײ�������Լ���ǰ�ߵ�ʱ���䵼�����Լ�ը��,��������취�����ã���Ϊ������İ��Լ�ը��
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,//Ҫ���ɵ�������Ч
			GetRootComponent(),//Ҫ�������ĸ������
			FName(),//Ҫ������������ĸ������,���ﲻ���ô���
			GetActorLocation(),//Ҫ���ɵ�λ����Ϣ
			GetActorRotation(),//Ҫ���ɵ���ת��Ϣ
			EAttachLocation::KeepWorldPosition,//Ҫ���ӵ�λ������
			false//�Ƿ�Ҫ�Զ����٣�����ϣ���ֶ�����
		);//�õ�����һ��������Ч���
	}
}


// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());//�ڱ����е�λ�÷���һ��ǹ�۵�����
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());//�ڱ����е�λ�ò���һ�������е���Ч
	}

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);//����3s�ӳ����ٱ�֤�������β�������ڻ����������ײ��ʱ����������

}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::ExplodeDamage()
{

	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)//Ӧ��һ�����η�Χ�˺�
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//���������Ķ��� 
				Damage,//�����˺�
				0.3 * Damage,//����˺�
				GetActorLocation(),//�ڻ��е�λ����Ϊ�˺�������
				DamageInnerRadius,//���˺�����
				DamageOuterRadius,//���˾���
				1.f,//�˺�˥��������ָ�����㣬����Ϊ1����Եõ�һ��������������˺�˥��
				UDamageType::StaticClass(),//�˺�����
				TArray<AActor*>(),//���Զ�ָ�����͵�������˺�
				this,//�����˺��Ķ���
				FiringController//˭�����
			);//������˥�������η�Χ�˺�
		}
	}
}

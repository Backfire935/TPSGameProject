// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShells.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
ABulletShells::ABulletShells()
{

	PrimaryActorTick.bCanEverTick = false;

	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellComp"));
	SetRootComponent(BulletShellMesh);

	BulletShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//����ͨ����ײ�����������
	BulletShellMesh->SetSimulatePhysics(true);//����ģ����������
	BulletShellMesh->SetEnableGravity(true);//������������
	BulletShellMesh->SetNotifyRigidBodyCollision(true);//ģ�����������¼�
	ShellEjectionImpulse = 10.f;//�����ӵ����׳��ĳ���

}


void ABulletShells::BeginPlay()
{
	Super::BeginPlay();
	
	BulletShellMesh->OnComponentHit.AddDynamic(this, &ABulletShells::OnHit);//���ӵ���ص��¼���ӵ�ί��

	BulletShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);//���ӵ������һ����ǰ�ĵ�λ����

	SetLifeSpan(5.f);//�����������5s����������
}

void ABulletShells::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BulletSound)//�������ӵ�����ص������������ӵ���
	{
		//BulletShellMesh->SetNotifyRigidBodyCollision(false);//ģ�����������¼�
		//BulletShellMesh->SetSimulatePhysics(true);//�ر�ģ����������
		UGameplayStatics::PlaySoundAtLocation(this, BulletSound, GetActorLocation());//���ӵ�����ص�λ�ò���һ�������е���Ч
		
	}
	BulletShellMesh->SetNotifyRigidBodyCollision(false);
	//Destroy();
}


void ABulletShells::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


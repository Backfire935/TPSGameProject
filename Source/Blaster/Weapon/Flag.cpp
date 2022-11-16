// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include"Components/SphereComponent.h"
#include"Components/WidgetComponent.h"


AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);//��USphereComponent���ӵ�������
	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void AFlag::Dropped()
{

	SetWeaponState(EWeaponState::Weapon_Dropped);//�Ƚ��˼�������״̬����Ϊ����״̬
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
	FlagMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
	SetOwner(nullptr);//��ӵ��������Ϊ��
	BlasterOwnerCharacter = nullptr;//��ӵ�д������Ľ�ɫ������Ϊ��
	BlasterOwnerController = nullptr;//��ӵ�д������Ŀ���������Ϊ��
}

void AFlag::HandleWeaponEquiped()
{
	ShowPickupWidget(false);//�ر�������ʰȡ��ʾ
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ر���������ײ����
	FlagMesh->SetSimulatePhysics(false);//����ģ������
	FlagMesh->SetEnableGravity(false);//������������
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void AFlag::HandleWeaponDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//������������ײ����
	}

	FlagMesh->SetSimulatePhysics(true);//����ģ������
	FlagMesh->SetEnableGravity(true);//������������
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //����������������ײ���ͨ������Ӧģʽ
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //���� ���Զ�Pawnͨ������ײ���
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //���� ���Զ�Cameraͨ������ײ���


}

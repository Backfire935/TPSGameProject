// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include"Components/SphereComponent.h"
#include"Components/WidgetComponent.h"
#include"Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "BulletShells.h"
#include "Engine/SkeletalMeshSocket.h"
#include"Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include  "WeaponTypes.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;//�������縴��
	SetReplicateMovement(true);//���ƶ�Ҳ�������縴��

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //����������������ײ���ͨ������Ӧģʽ
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //���� ���Զ�Pawnͨ������ײ���
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();//������ǵ�ǰ����Ⱦ״̬Ϊ����Ⱦ
	EnableCustomDepth(true);

	SetRootComponent(WeaponMesh);//��WeaponMesh����Ϊ���ڵ�

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);//������������Ⱦ�Զ������

	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if ( PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}


// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon , WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);//��������ȵ��������ӵĺ���
	}
}


void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);//�ȵ��������Ӻ����Ķ�������Ϊ��
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if(BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{//ȷ��Ŀǰ�õĵ�ҩHUD�������õ������ĵ�ҩHUD�����Ǳ��ϱ���
			SetHUDAmmo();
		}
	}
}

void AWeapon::OnRep_Ammo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotGunEnd();//
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1 , 0 , MagCapacity);//��ammo������0����󱸵���֮��
	SetHUDAmmo();

}

void AWeapon::SetWeaponState(EWeaponState State)//���ص�
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()//���Ƶ�
{
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)//�ж�������״̬
	{
	case EWeaponState::Weapon_Equipped://��������ǳ���װ��״̬
	{
		HandleWeaponEquiped();
		break;
	}

	case EWeaponState::Weapon_EquippedSecondary :
		{
		HandleWeaponSecondary();
		break;
		}

	case EWeaponState::Weapon_Dropped://�������������������
		HandleWeaponDropped();
		break;
	}
}


void AWeapon::HandleWeaponEquiped()
{
	ShowPickupWidget(false);//�ر�������ʰȡ��ʾ
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ر���������ײ����
	WeaponMesh->SetSimulatePhysics(false);//����ģ������
	WeaponMesh->SetEnableGravity(false);//������������
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//����ǹ����ģ������
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//������������
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);//�ر������������
}

void AWeapon::HandleWeaponDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//������������ײ����
	}

	WeaponMesh->SetSimulatePhysics(true);//����ģ������
	WeaponMesh->SetEnableGravity(true);//������������
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //����������������ײ���ͨ������Ӧģʽ
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //���� ���Զ�Pawnͨ������ײ���
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //���� ���Զ�Cameraͨ������ײ���

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();//������ǵ�ǰ����Ⱦ״̬Ϊ����Ⱦ
	EnableCustomDepth(true);

}

void AWeapon::HandleWeaponSecondary()
{
	ShowPickupWidget(false);//�ر�������ʰȡ��ʾ
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ر���������ײ����
	WeaponMesh->SetSimulatePhysics(false);//����ģ������
	WeaponMesh->SetEnableGravity(false);//������������
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//����ǹ����ģ������
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//������������
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(true);//�ر������������
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();//������ǵ�ǰ����Ⱦ״̬Ϊ����Ⱦ
	}
}

bool AWeapon::IsEmpty()//�ж��ӵ��Ƿ����
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;//��ǰʣ���ӵ������ǲ��ǵ��ڵ�������
}

FVector AWeapon::TraceWithScatter(const FVector& HitTarget)//����ɢ������߼��
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return FVector();//һ��ֱ�������������

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();//���������ʼ��

	FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//һ����������ʼ�㵽������Ŀ�������
	FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//����������յ���е�����
	FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//�������λ����*������� 
	FVector EndLoc = SphereCenter + RandVec;//���ĵ����ܵ������ɢ����
	FVector ToEndLoc = EndLoc - TraceStart;//�������߶�
	FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//������������

	/*
	 *DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 10.f);//����ɢ����ɢ��Χ
	DrawDebugSphere(GetWorld(), EndLoc, 4.F, 12, FColor::Blue, false,10.f);//���������ӵ������
	DrawDebugLine(GetWorld(), TraceStart, EndEnd, FColor::Orange, false, 10.f);//���������ӵ������
*/

	return EndEnd;//������������
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	//�������������Ч��
	if (FireAnimation)//������𶯻����ڵĻ�,������ģ�Ͳ���������𶯻�
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);//�ڶ���������ѭ�����ŵĲ���������Ϊfalse
	}

	//�������ɲ��׳��ӵ���
	if (BulletShells)//����ӵ������Ǵ��ڵĻ�
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));//��ȡ����ǹ�ڵĲ��
		if (AmmoEjectSocket)//ǹ�ڲ�۵�λ��
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());//��ȡ��ǰ���ϵ�������ǹ�ڲ�۵�λ�ú���ת��Ϣ��������ӵ����׳��ķ��򣬾�����Ҫ��ǹе�Ĺ��������ò��λ�ú���ת�Ƕ�
			UWorld* World = GetWorld();//��ȡ���糡��
			if (World)
			{
				World->SpawnActor<ABulletShells>(//ֱ�������糡���������ӵ��ǣ������Ǿ�������
					BulletShells,//���ɵ��������ӵ���
					SocketTransform.GetLocation(),//���ɵ�λ����������۵�λ��
					SocketTransform.GetRotation().Rotator()	//���ɵ���ת�Ƕ��ǲ�۵���ά��ת��Ϣ
					);
			}
		}
	}//�ӵ����׳�Ч������

	if(HasAuthority())
	{
		SpendRound();
	}

}

void AWeapon::Dropped()
{
	bDestroyedWeapon = true;//�����������䵽����׼�����ٵļ�ʱ
	ReadyDestroyWeapon();
	SetWeaponState(EWeaponState::Weapon_Dropped);//�Ƚ��˼�������״̬����Ϊ����״̬
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
	WeaponMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
	SetOwner(nullptr);//��ӵ��������Ϊ��
	BlasterOwnerCharacter = nullptr;//��ӵ�д������Ľ�ɫ������Ϊ��
	BlasterOwnerController = nullptr;//��ӵ�д������Ŀ���������Ϊ��
}

void AWeapon::ReadyDestroyWeapon()
{
	GetWorldTimerManager().SetTimer(
		DestroyWeaponTimer,
		this,
		&AWeapon::DestroyWeapon,
		DestroyWeaponTime
	);
}

void AWeapon::DestroyWeapon()
{
	//���֮ǰ���������������Ļ�����ôEquippedWeapon��StoryedToDestroyWeaponָ��ָ�����ͬһ��ʵ������
	if (bDestroyedWeapon)//������ǰ����һ�μ�飬����ʱ�����Ѿ���ʰȡ�������򲻻���������
	{
		 Destroy();
	}
}
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo -	AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}


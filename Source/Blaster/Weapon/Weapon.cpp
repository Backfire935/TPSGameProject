﻿// Fill out your copyright notice in the Description page of Project Settings.


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

	bReplicates = true;//开启网络复制
	SetReplicateMovement(true);//将移动也开启网络复制

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //设置武器对所有碰撞检测通道的响应模式
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //覆盖 忽略对Pawn通道的碰撞检测
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();//用来标记当前的渲染状态为已渲染
	EnableCustomDepth(true);

	SetRootComponent(WeaponMesh);//将WeaponMesh设置为根节点

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
		WeaponMesh->SetRenderCustomDepth(bEnable);//开启武器的渲染自定义深度

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
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);//将自身传入踩到武器盒子的函数
	}
}


void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);//踩到武器盒子函数的对象设置为空
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
		{//确保目前用的弹药HUD是手上拿的武器的弹药HUD而不是背上背的
			SetHUDAmmo();
		}
	}
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


void AWeapon::SpendRound()//打完了更新下子弹
{
	Ammo = FMath::Clamp(Ammo - 1 , 0 , MagCapacity);//让ammo限制在0到最大备弹数之间
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else  
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotGunEnd();
	}

}

void AWeapon::SetWeaponState(EWeaponState State)//本地的
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()//复制的
{
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)//判断武器的状态
	{
	case EWeaponState::Weapon_Equipped://如果武器是出于装备状态
	{
		HandleWeaponEquiped();
		break;
	}

	case EWeaponState::Weapon_EquippedSecondary :
		{
		HandleWeaponSecondary();
		break;
		}

	case EWeaponState::Weapon_Dropped://如果武器被丢到地上了
		HandleWeaponDropped();
		break;
	}
}


void AWeapon::HandleWeaponEquiped()
{
	ShowPickupWidget(false);//关闭武器的拾取提示
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//关闭武器的碰撞盒子
	WeaponMesh->SetSimulatePhysics(false);//开启模拟物理
	WeaponMesh->SetEnableGravity(false);//开启武器重力
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//开启枪带的模拟物理
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//开启武器重力
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);//关闭武器轮廓描边

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::HandleWeaponDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//开启武器的碰撞盒子
	}

	WeaponMesh->SetSimulatePhysics(true);//开启模拟物理
	WeaponMesh->SetEnableGravity(true);//开启武器重力
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //设置武器对所有碰撞检测通道的响应模式
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //覆盖 忽略对Pawn通道的碰撞检测
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //覆盖 忽略对Camera通道的碰撞检测

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();//用来标记当前的渲染状态为已渲染
	EnableCustomDepth(true);

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter )
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::HandleWeaponSecondary()
{
	ShowPickupWidget(false);//关闭武器的拾取提示
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//关闭武器的碰撞盒子
	WeaponMesh->SetSimulatePhysics(false);//开启模拟物理
	WeaponMesh->SetEnableGravity(false);//开启武器重力
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//开启枪带的模拟物理
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//开启武器重力
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(true);//关闭武器轮廓描边
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();//用来标记当前的渲染状态为已渲染
	}
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;

	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

bool AWeapon::IsEmpty()//判断子弹是否打完
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;//当前剩余子弹数量是不是等于弹夹容量
}

FVector AWeapon::TraceWithScatter(const FVector& HitTarget)//喷子散射的射线检测
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return FVector();//一个直线武器攻击检测

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();//开火检测的起始点

	const FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//一个从射线起始点到被击中目标的向量
	const FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//到喷子射程终点的中点向量
	const FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//随机方向单位向量*随机长度 
	const FVector EndLoc = SphereCenter + RandVec;//中心到四周的随机扩散向量
	const FVector ToEndLoc = EndLoc - TraceStart;//两点间的线段

	FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//单个射线向量

	/*
	 *DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 10.f);//整个散射扩散范围
	DrawDebugSphere(GetWorld(), EndLoc, 4.F, 12, FColor::Blue, false,10.f);//单个喷子子弹的落点
	DrawDebugLine(GetWorld(), TraceStart, EndEnd, FColor::Orange, false, 10.f);//单个喷子子弹的落点
*/

	return EndEnd;//单个射线向量
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
	//武器产生开火的效果
	if (FireAnimation)//如果开火动画存在的话,由武器模型播放这个开火动画
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);//第二个参数是循环播放的参数，设置为false
	}

	//武器生成并抛出子弹壳
	if (BulletShells)//如果子弹壳类是存在的话
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));//获取武器枪口的插槽
		if (AmmoEjectSocket)//枪口插槽的位置
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());//获取当前手上的武器的枪口插槽的位置和旋转信息，这就是子弹壳抛出的方向，具体需要在枪械的骨骼中设置插槽位置和旋转角度
			UWorld* World = GetWorld();//获取世界场景
			if (World)
			{
				World->SpawnActor<ABulletShells>(//直接在世界场景中生成子弹壳，下面是具体设置
					BulletShells,//生成的物体是子弹壳
					SocketTransform.GetLocation(),//生成的位置是武器插槽的位置
					SocketTransform.GetRotation().Rotator()	//生成的旋转角度是插槽的三维旋转信息
					);
			}
		}
	}//子弹壳抛出效果结束

		SpendRound();//打完了更新下子弹

}

void AWeapon::Dropped()
{
	bDestroyedWeapon = true;//进行武器掉落到地上准备销毁的计时
	ReadyDestroyWeapon();
	SetWeaponState(EWeaponState::Weapon_Dropped);//先将此件武器的状态的设为丢弃状态
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
	WeaponMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
	SetOwner(nullptr);//将拥有者设置为空
	BlasterOwnerCharacter = nullptr;//将拥有此武器的角色类设置为空
	BlasterOwnerController = nullptr;//将拥有此武器的控制器设置为空
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
	//如果之前的武器被捡起来的话，那么EquippedWeapon和StoryedToDestroyWeapon指针指向的是同一个实例武器
	if (bDestroyedWeapon)//在销毁前进行一次检查，若此时武器已经被拾取起来，则不会销毁武器
	{
		 Destroy();
	}
}



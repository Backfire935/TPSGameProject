// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include"Net/UnrealNetwork.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include"DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include"Sound/SoundCue.h"
#include "Blaster/Weapon/Projectile.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;//设置为每帧执行
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 400.f;

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);//注册CombatComponent类中声明的AWeapon类型的EquippedWeapon变量

	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);//注册第二把武器

	DOREPLIFETIME(UCombatComponent, bAiming);//注册CombatComponent类中声明的bool类型的bAiming变量

	DOREPLIFETIME(UCombatComponent, CombatState);

	DOREPLIFETIME(UCombatComponent, Grenades);

	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly); //注册本类中声明的int32类型的备弹量变量的网络同步
}

void UCombatComponent::PickupAmmp(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))//确保是有这种武器类型的
	{
		CarriedAmmoMap[WeaponType] += AmmoAmount;//给指定的武器类型加备弹

		UpdateCarriedAmmo();
	}
	if(EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();//如果武器没子弹了且此时捡到了子弹，自动装填
	}


}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;//控制速度
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;//获取相机视野
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())//控制备弹
		{
			InitializeCarriedAmmo();//初始化备弹
		}
	}

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime); 
		InterpFOV(DeltaTime);
	}
	
	// ...
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr ) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutoMatic)//如果这里不允许全自动，第二次会直接停在这里
	{
		Fire();
	}

	ReloadEmptyWeapon();
}

void UCombatComponent::OnRep_EquippedWeapon()//RPC调用其他客户端模拟走路姿态的开关
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);

		AttachActorToRightHand(EquippedWeapon);//东西放右手上

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;//关闭角色向移动的方向旋转
		Character->bUseControllerRotationYaw = true; //开启角色跟随鼠标的左右旋转

		PlayEquipWeaponSound(EquippedWeapon);//播放装备武器的音效
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);//把第二把武器装后背包上
		PlayEquipWeaponSound(SecondaryWeapon);//播放装备武器的音效
		SecondaryWeapon->EnableCustomDepth(true);
	
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)//按下开火按钮后
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed && EquippedWeapon)//如果不加检查武器会导致点击窗口的时候因为没有武器而导致bPressed=false而无法开火
	{
		Fire();
	}

}

void UCombatComponent::ShotGunShellReload()//这个函数是用来暴露给蓝图用的
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();//每次+1子弹
	}
	
}



void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;

		if (EquippedWeapon)//增加开火准心散布
		{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				{
				FireProjectileWeapon();
				break;
				}
			case EFireType::EFT_HitScan:
				{
				FireHitScanWeapon();
					break;
				}
			case EFireType::EFT_Shotgun:
				{
				FireShotgunWeapon();
					break;
				}

				default:
					break;
			}

		}
		StartFireTimer();
	}	
}

void UCombatComponent::FireProjectileWeapon()
{
	LocalFire(HitTarget);
	ServerFire(HitTarget);
}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireShotgunWeapon()
{

}


bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)//武器为喷子并且在换弹的情况下
	{
		return true;//允许开火
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;//子弹不为0且允许开火

}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//设置备弹HUD
	}
	bool bJumpToShotGunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
	if(bJumpToShotGunEnd)
	{
			//如果装满了就直接跳到装填完毕的动画
			JumpToShotGunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);

}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)//从GEngine获取视口
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);//获取游戏屏幕的大小

	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.0F, ViewportSize.Y / 2.0f);//屏幕大小除以二就是中心点的坐标,不过是屏幕坐标，不是世界坐标

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//将给定的2D屏幕空间坐标转换为三维世界空间点和方向
	bool bScreenToWorld = 	UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);//运行后rosshairWorldPosition,CrosshairWorldDirection被填充数据

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;//子弹射出去开始的位置

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TraceLength;//子弹射出去停止的位置，相当于最大射程

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())//检查射线检测是否碰撞到了物体并且是否继承了接口
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;//瞄中人了准心变红
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;//没瞄中人准心变白
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)//服务器开火
{
	MultiCastFire(TraceHitTarget);//在服务器上执行多播会在服务器和所有的客户端上执行
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)//服务器发起的向客户端的多播开火
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;//如果接到通知的是本机，就不执行这句话，因为之前执行过了
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)//如果武器为喷子且正在换弹的话
	{
		//开火需要播放两种动画，一种是角色的开火动作动画，还有一种是武器的动作动画
		Character->PlayFireMontage(bAiming);//角色类播放开火蒙太奇动画
		EquippedWeapon->Fire(TraceHitTarget);//武器类触发开火事件
		CombatState = ECombatState::ECS_Unoccupied;//将武器状态重新设置为 普通状态
		return; //空返回 因为不需要执行后面的了
	}

	if (Character && CombatState == ECombatState::ECS_Unoccupied)//如果角色类存在，且武器状态是空闲状态的话
	{
		//开火需要播放两种动画，一种是角色的开火动作动画，还有一种是武器的动作动画
		Character->PlayFireMontage(bAiming);//角色类播放开火蒙太奇动画
		EquippedWeapon->Fire(TraceHitTarget);//武器类触发开火事件
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;//检查角色和武器是否为空
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)//如果已经装备了一把武器并且没装第二把，就可以装第二把
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);//不然就是装第一件武器
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;//关闭角色向移动的方向旋转
	Character->bUseControllerRotationYaw = true; //开启角色跟随鼠标的左右旋转
}

void UCombatComponent::SwapPrimaryWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	//设置主武器的部分,副武器掏出来变成主武器
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//设置武器的状态为已装备
	AttachActorToRightHand(EquippedWeapon);//东西放右手上
	EquippedWeapon->SetOwner(Character);//设置所有权
	EquippedWeapon->SetHUDAmmo();//设置当前弹药HUD
	UpdateCarriedAmmo();//更新携带的弹药
	PlayEquipWeaponSound(EquippedWeapon);//播放捡起武器的声音
	ReloadEmptyWeapon();//武器要是空的就装子弹
	//设置副武器的部分

	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//设置武器的状态为已装备
	AttachActorToBackpack(SecondaryWeapon);//主武器放到后背去

}

void UCombatComponent::SwapSecondaryWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	AWeapon* TempWeapon = SecondaryWeapon;
	SecondaryWeapon = EquippedWeapon;
	EquippedWeapon = TempWeapon;

	//设置主武器的部分,副武器掏出来变成主武器
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//设置武器的状态为已装备
	AttachActorToRightHand(EquippedWeapon);//东西放右手上
	EquippedWeapon->SetOwner(Character);//设置所有权
	EquippedWeapon->SetHUDAmmo();//设置当前弹药HUD
	UpdateCarriedAmmo();//更新携带的弹药
	PlayEquipWeaponSound(EquippedWeapon);//播放捡起武器的声音
	ReloadEmptyWeapon();//武器要是空的就装子弹
	//设置副武器的部分

	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//设置武器的状态为已装备
	AttachActorToBackpack(SecondaryWeapon);//主武器放到后背去

}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();//如果手上有东西就把手上的东西丢了
	EquippedWeapon = WeaponToEquip;//装备的武器设置为传入的武器
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//设置武器的状态为已装备
	EquippedWeapon->bDestroyedWeapon = false;//不进行武器掉落到地上后准备销毁的计时
	AttachActorToRightHand(EquippedWeapon);//东西放右手上
	EquippedWeapon->SetOwner(Character);//设置所有权
	EquippedWeapon->SetHUDAmmo();//设置当前弹药HUD
	UpdateCarriedAmmo();//更新携带的弹药
	PlayEquipWeaponSound(WeaponToEquip);//播放捡起武器的声音
	ReloadEmptyWeapon();//武器要是空的就装子弹
	EquippedWeapon->EnableCustomDepth(false);//关闭武器轮廓描边
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;//装备的武器设置为传入的武器
	SecondaryWeapon->bDestroyedWeapon = false;//初始化或不进行武器掉落到地上后准备销毁的计时
	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//设置武器的状态为已装备
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);//播放捡起武器的声音
	SecondaryWeapon->EnableCustomDepth(true);//开启武器轮廓描边
	SecondaryWeapon->SetOwner(Character);//设置所有权

}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)//如果武器存在
	{
		EquippedWeapon->Dropped();//如果已经装备了一件武器，就丢掉手上的
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));//获取角色后背背包的插槽
		if(BackpackSocket)
		{
			BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
		}

}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));//获取角色模型身上的指定插槽
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//参数：武器，角色模型
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr || EquippedWeapon == nullptr) return;
	
	switch (EquippedWeapon->GetWeaponType())
	{
	case  EWeaponType::EWT_Pistol :
		{
		SelectWeaponSocket(FName("PistolSocket"));
			break;
		}

	case  EWeaponType::EWT_SMG :
		{
		SelectWeaponSocket(FName("PistolSocket"));
		break;
		}

	default:
		{
		SelectWeaponSocket(FName("LeftHandSocket"));
			break;
		}

	}

}

void UCombatComponent::SelectWeaponSocket(FName SocketName)
{
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName(SocketName));//获取角色模型身上的指定插槽
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//参数：武器，角色模型
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if(Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}



void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//检查武器类型
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];//从map中获取当前武器种类的备弹数
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//设置备弹HUD
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip ==nullptr || WeaponToEquip->EquipSound == nullptr) return;
	//播放装备武器的音效
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (Character && EquippedWeapon && EquippedWeapon->IsEmpty())//如果捡起来的武器是空的就自动换弹
	{
		Reload();
	}

}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)//设置十字准心
{
	if (Character == nullptr || Character->Controller == nullptr) return;//先判定角色存不存在

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)//再判断控制器存不存在
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)//再判断HUD存不存在
		{
			if (EquippedWeapon)//再判断是否装备了武器
			{//要是存在HUD类就从装备的武器类那拿到预设的准心
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{//要是没武器就设空，就没有准心了
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			//建立一个速度区间到扩散量的映射[0,600]->[0,1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityOutputRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityOutputRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			//设置准心扩散的量
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CrosshairAimTarget, DeltaTime, 30.f);//瞄准的时候，准星变小变准
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);//没瞄准的时候，准心扩散为0(不考虑开枪和空中)
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);//让瞄准准星扩散值快速恢复到0

			HUDPackage.CrosshairsSpread =
				0.5 +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);

		}
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();//弹夹中的余下空间 = 满弹 - 剩余子弹

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//检查武器类型
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);//取RoomInMag和AmountCarried之间最小的变量
		return FMath::Clamp(RoomInMag, 0, Least);//使RoomInMag
	}
	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	if (Character && Character->GetAttachedGrenade())
	{
		ShowAttachedGrenade(true);
		CombatState = ECombatState::ECS_ThrowingGrenade;
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}
	if(Character && !Character->HasAuthority() && Character->GetAttachedGrenade())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);//这里进行手雷数量的减少
		UpdateHUDGrenades();
	}
}



void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		ShowAttachedGrenade(true);
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);//这里进行手雷数量的减少
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);

	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading && !EquippedWeapon->IsFull())//还有子弹且不是满弹且当前武器状态不处于换弹才能够换弹
	{
		ServerReload();
	}
}

void UCombatComponent::FinishReload()//由蓝图直接调用的函数
{
	if (Character == nullptr)  return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
		UE_LOG(LogTemp, Warning, TEXT("FinishReload"));
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerReload_Implementation()//服务端上呼叫HandleReload
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;//切换武器状态到换弹
	HandleReload();
}

void UCombatComponent::OnRep_CombatState()//客户端上呼叫HandleReload
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			ShowAttachedGrenade(true);
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//检查武器类型
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//设置备弹HUD
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);

}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//检查武器类型
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//设置备弹HUD
	}
	EquippedWeapon->AddAmmo(-1);//一旦加了子弹，肯定就能打了
	bCanFire = true;

	
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0 )
	{
		//如果装满了就直接跳到装填完毕的动画
		JumpToShotGunEnd();
	}
}



void UCombatComponent::JumpToShotGunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));//直接跳转到制定的蒙太奇动画
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LauncherGrenade()
{
	ShowAttachedGrenade(false);
	if(Character && Character->IsLocallyControlled())
	{
		ServerLuncherGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLuncherGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character  && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParameters
				);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)//FOV的插值变换
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());//对处于瞄准状态的FOV进行设置
	}
	else
	{//
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);//对非瞄准状态的FOV进行设置
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);//设置为当前的FOV
	}
}

void UCombatComponent::SetAiming(bool IsAiming)//设置是否瞄准
{
	if(Character == nullptr || EquippedWeapon == nullptr) return;

	bAiming = IsAiming;//其实不管怎么样 bAiming都是本地的且replicate的，所以本地会首先被设置然后经过一段延迟后在潜在的服务器上运行并replicate到其他客户端
	ServerSetAiming(IsAiming);//在客户端上本地设置后再去服务器端调用这个函数再replicate到其他客户端，在服务器上的话，就正好本地调用一次再replicate
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed; //在瞄准的话就改变角色移动速度
	}

	if(Controller->IsLocalController() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)//如过手上拿的是狙就换一种开镜
	{
		Character->ShowSniperScopeWidget(IsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)//瞄准的服务器端RPC调用
{
	bAiming = bIsAiming;//其实不管怎么样 bAiming都是本地的且replicate的，所以本地会首先被设置然后经过一段延迟后在潜在的服务器上运行并replicate到其他客户端
	if (Character)//进行一次RPC调用
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed; //在瞄准的话就改变角色移动速度
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include <string>
#include "GameFramework/SpringArmComponent.h"//弹簧臂组件的库文件
#include "Camera/CameraComponent.h"	//摄像机组件的库文件
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include"Blaster/Weapon/Weapon.h"
#include"Blaster/BlasterComponents/CombatComponent.h"
#include"Blaster/BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include"Kismet/KismetMathLibrary.h"
#include"Kismet/GameplayStatics.h"
#include "BlasterAnimInstance.h"
#include"Blaster/Blaster.h"
#include"Blaster/PlayerController/BlasterPlayerController.h"
#include"Blaster/GameMode/BlasterGameMode.h"
#include"TimerManager.h"
#include"Sound/SoundCue.h"
#include"Particles/ParticleSystemComponent.h"
#include"Blaster/PlayerState/BlasterPlayerState.h"
#include"Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"

// Sets default values 
ABlasterCharacter::ABlasterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;//角色生成设置:尽可能的调整位置，但若还是碰撞也依然生成

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //创建弹簧臂并命名为CameraBoom
	CameraBoom->SetupAttachment(GetMesh());	//弹簧臂依附于网格体组件上
	CameraBoom->TargetArmLength = 600.0f;		//弹簧臂目标臂长度设为600
	CameraBoom->bUsePawnControlRotation = true;   //使用pawn控制旋转

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false; //角色朝向控制器方向，即鼠标
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);//开启combat的网络复制

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//角色移动组件设置角色默认能够下蹲，蓝图中在角色蓝图的character movement组件找crouch设置
	GetCharacterMovement()->RotationRate =FRotator(0.f,0.f,850.f);//设置角色转向速率

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//设置对相机组件的碰撞忽略
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);//将人物的碰撞类型设置为自定义的宏SkeletalMesh类型
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//设置对相机组件的碰撞忽略
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//设置对射线检测的碰撞


	TurningInPlace = ETurningInPlace::ETIP_NotTurning;//上来先初始化下旋转方向，让它别动

	NetUpdateFrequency = 66.f;//网络更新频率，每秒66次
	MinNetUpdateFrequency = 33.f;//最小网络更新频率，每秒33次

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//
	//身体的hitbox
	//
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	head->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	blanket->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const//replicate属性注册
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);//头文件#include "Net/UnrealNetwork.h"
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
	
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;
	
	}
}//请求初始化组件

void ABlasterCharacter::PlayFireMontage(bool bAiming)//播放开火的蒙太奇动画
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);//播放开火的蒙太奇动画
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//选定是开镜的蒙太奇还是没开镜的
		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到制定的蒙太奇动画
	}
}

void ABlasterCharacter::PlayReloadMontage()//播放重装弹夹的动画蒙太奇
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)//检查武器组件是否为空，同时组件中是否存在已经装备的武器
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//获取角色的模型再获取动画实例
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);//播放装弹的蒙太奇动画
		FName SectionName;
		
		switch (Combat->EquippedWeapon->GetWeaponType())//选择武器的种类并播放动画，添加新的武器后，在此处添加代码
		{
		case EWeaponType::EWT_AssaultRifle :
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;

		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_ShotGun:
			SectionName = FName("ShotGun");
			break;

		case EWeaponType::EWT_Sniper:
			SectionName = FName("Sniper");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到制定的蒙太奇动画
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);//播放升天的蒙太奇动画
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);//播放扔手雷的蒙太奇动画
	}
}

void ABlasterCharacter::PlayHitReactMontage()//播放被击中后的受击效果动画蒙太奇
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}//会检查被命中的人是否持有武器，若无武器则不会播放受击动画
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
	
		AnimInstance->Montage_Play(HitReactMontage);//播放开火的蒙太奇动画
		FName SectionName("FromForward");
	//	SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//选定是开镜的蒙太奇还是没开镜的
		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到制定的蒙太奇动画
	}
}

void ABlasterCharacter::TabButtonPressed()
{
	
}

void ABlasterCharacter::TabButtonReleased()
{

}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if(bElimed) return;//如果自己已经死了就不会再受到伤害了

	float DamageToHealth = Damage;
	float NotHitedShield = 0.f;
	if(Shield>0)
	{
		if(DamageToHealth > Shield)//伤害比护盾高，破防扣血
		{
			NotHitedShield = Shield;//存储一次还没收到攻击时的护盾值
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//设置新的护盾值和受到的伤害
			DamageToHealth = DamageToHealth - NotHitedShield;//设置对血条造成的伤害为扣除护盾抵挡后的伤害

		}
		else//伤害比护盾低，只有护盾降低
		{
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//设置护盾值和受到的伤害
			DamageToHealth = 0.f;//本次攻击不会对血条造成影响
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);//设置生命值和受到的伤害
	UpdateHUDShield();
	UpdateHUDHealth();

	PlayHitReactMontage();
	if (Health == 0.f)//如果血量到0,进行淘汰
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController );
		}
	}


}

void ABlasterCharacter::OnRep_ReplicatedMovement()//将角色转向应用到模拟代理
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();//模拟代理的朝向
	TimeSinceLastMovementReplication = 0.f;//将时间计时器重置为0
	
}

void ABlasterCharacter::Elim()//只会在服务器上调用
{
	DropOrDestroyWeapons();
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyedWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}


void ABlasterCharacter::MulticastElim_Implementation()//玩家淘汰时
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);//死亡后设置HUD子弹数量为0
	}
	bElimed = true;
	PlayElimMontage();

	//开始溶解效果
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//禁用角色移动
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	/*if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}禁用玩家输入*/
	bDisableGameplay = true;//直接将此项设置为true，则禁用相关项的输入响应
	GetCharacterMovement()->DisableMovement();//击杀玩家之后禁用了移动和重力防止玩家掉入虚空
	if(Combat)
	{
		Combat->FireButtonPressed(false);//如果玩家这个时候有武器且在按开火键就关掉
	}
	//禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//生成淘汰音效
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 150.f);
		ElimBotComponent =  UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
			);
	}
	if (ElimBotSound)
	{//在生成的位置播放淘汰音效
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::ElimTimerFinished()//淘汰计时器结束之后,从游戏模式那复活玩家
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);//请求复活玩家
	}

}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)//只有在重置或者转换关卡的时候才销毁武器
	{
		Combat->EquippedWeapon->Destroyed();
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();//生成初始武器，具体哪把需要在蓝图中设置
	UpdateHUDAmmo();//拿到初始武器后设置HUD弹药
	UpdateHUDHealth();//更新生命值HUD
	UpdateHUDShield();//更新护盾HUD
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	if(AttachedGrenade)//初始化设置手雷的可视性为fasle
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);//控制角色转向的函数

	HideCameraIfCharacterClose();//如果角色和相机的距离过近则隐藏角色和枪械的模型
	PollInit();//初始化BlasterPlayerState

}

void ABlasterCharacter::RotateInPlace(float DeltaTime)//控制角色转向的函数
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())//Enum特性,ROLE_None值为0,ROLE_SimulatedProxy值为1,ROLE_AutonomousProxy为2,其他更大
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;//计时器
		if (TimeSinceLastMovementReplication > 0.1f)//达到0.1s就调用
		{
			OnRep_ReplicatedMovement();//将角色转向应用到模拟代理
		}
		CalculateAO_Pitch();//计算围绕pitch的旋转
	}
}
void ABlasterCharacter::MoveForward(float Value)//向前移动
{

	if(bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,  Controller->GetControlRotation().Yaw, 0.f); //方向
		const FVector   Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));//X向量
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)//向右移动
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //方向
		const FVector   Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));//Y向量
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)//鼠标Yaw转向
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)//鼠标Pitch转向
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()//E键的响应
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Combat)
	{
		if (HasAuthority())//如果在服务器中
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//若是在客户机上触发了，则进行RPC 
		{
			ServerEquipButtonPressed();
		}
	}

}

void ABlasterCharacter::CrouchButtonPressed()//下蹲的响应
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (bIsCrouched)
		UnCrouch();
	else
	Crouch();
}

void ABlasterCharacter::AimButtonPressed()//鼠标右键按下的响应
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if(Combat)
	{ 
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()//鼠标右键松开的响应
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::SwapPrimaryWeapon()
{
	if(Combat->EquippedWeapon && Combat->SecondaryWeapon)
	Combat->SwapPrimaryWeapon();
}

void ABlasterCharacter::SwapSecondaryWeapon()
{
	if (Combat->EquippedWeapon && Combat->SecondaryWeapon)
	Combat->SwapSecondaryWeapon();
}

void ABlasterCharacter::DropCurrentWeapon()//按G丢武器
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->SecondaryWeapon)//丢掉之后如果身上有第二把武器，自动切换到第二把武器
		{
			Combat->EquippedWeapon->Dropped();//先把手上的丢了
			Combat->EquippedWeapon = Combat->SecondaryWeapon;//将要装备的武器设置为第二把武器
			UE_LOG(LogTemp, Warning, TEXT("test"));
			//设置要使用的武器的状态
			Combat->EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//设置武器的状态为已装备
			Combat->AttachActorToRightHand(Combat->EquippedWeapon);//东西放右手上
			Combat->EquippedWeapon->SetOwner(this);//设置所有权
			Combat->EquippedWeapon->SetHUDAmmo();//设置当前弹药HUD
			Combat->UpdateCarriedAmmo();//更新携带的弹药
			Combat->PlayEquipWeaponSound(Combat->EquippedWeapon);//播放捡起武器的声音
			Combat->ReloadEmptyWeapon();//武器要是空的就装子弹

			//将没有用的第二把武器的指针设为空，现在就只有一把武器了
			Combat->SecondaryWeapon = nullptr;//因为模型还在，这个时候销毁后背上的模型,这个时候角色只有主武器没有副武器
		}//如果没有的话，将自身的状态切换至空
		else//如果玩家只有主武器的话
		{
			Combat->EquippedWeapon->Dropped();//枪丢了就行
			Combat->EquippedWeapon = nullptr;//指针置空
			GetCharacterMovement()->bOrientRotationToMovement = true;//开启角色向移动的方向旋转
			bUseControllerRotationYaw = false; //关闭角色跟随鼠标的左右旋转
		}
	}
	

}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}



float ABlasterCharacter::CalculateSpeed()
{
	FVector  Velocity = GetVelocity();
	Velocity.Z = 0.f;

	return Velocity.Size(); //获取三维向量的模长，其中Z为0
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;//如果手上没武器直接返回空



	float Speed = CalculateSpeed(); //获取三维向量的模长，其中Z为0

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)//不在空中不在动
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//每帧存储Yaw
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		//bUseControllerRotationYaw = false;//关闭角色朝向旋转

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;//关闭角色朝向旋转

		TurnInPlace(DeltaTime);//进行角色朝向的超标修正，见课61
	}

	if (Speed > 0.f || bIsInAir)//跑步或者跳的时候
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//每帧存储Yaw
		AO_Yaw = 0.f;	
		bUseControllerRotationYaw = true;//开启角色朝向旋转
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();//计算围绕pitch的旋转

}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90 && !IsLocallyControlled())//处理别的客户端发来的AO_Pitch数值转换错误问题，详情见课58
	{
		//从[270,360)映射到[-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);

	}
}

void ABlasterCharacter::SimProxiesTurn()//模拟代理的朝向
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw =  UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;//计算上一帧到现在的旋转差值
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)//修改模拟代理角色的朝向
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

void ABlasterCharacter::Jump()//重写跳跃，如果是蹲伏的，按空格也可恢复站立
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{
		Super::Jump();//执行父类的Jump函数
	}
}

void ABlasterCharacter::FireButtonPressed()//按下开火键
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()//松开开火键
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.F)//向右旋转角度超过90度
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}

	else if (AO_Yaw < -90.F)//向左旋转角度超过90度
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);//插值缓入
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)//旋转度数不大的话
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//每帧存储Yaw

		}
	}
		
	
}
	
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)//客户端的武器拾取提示界面Rep_Notify
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()//服务端的武器碰撞盒子响应
{
	if (Combat)//如果还在武器碰撞盒子范围内，就拾取
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}

}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)//当角色和相机组件的距离过近时，让角色在本地消失从而不影响视野
	{
		GetMesh()->SetVisibility(false);//设置人物不可见	
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//设置拥有者不可见
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);//设置人物可见	
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//设置拥有者可见
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(Health < LastHealth)//现在的血量比之前的低，说明受到了伤害
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)//现在的护盾比之前的低，说明受到了伤害
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()//开始溶解
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)//武器拾取界面的提示
{
	if (OverlappingWeapon)//如果已经有了武器，就关掉拾取提示
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;//传进来碰到的武器

	if (IsLocallyControlled())//武器所有权在本地的情况下，即未被其他人拾取的话
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);//打开武器拾取界面
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()//检查是否装备武器
{
	return (Combat && Combat->EquippedWeapon);//Combat对象不为空且EquippedWeapon为true才能返回true
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_Max;
	return Combat->CombatState;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;

}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
		
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);

	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld *World = GetWorld();
	if(BlasterGameMode && World && !bElimed && DefaultWeaponClass)
	{
		AWeapon * StartingWeapon =  World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if(Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}

	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRight", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);



	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);//E键
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);//Ctrl键
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);//鼠标右键按下触发函数
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);//鼠标右键松开触发函数
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);//开火按钮按下触发函数
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);//开火按钮松开触发函数

	PlayerInputComponent->BindAction("TabList", IE_Pressed, this, &ABlasterCharacter::TabButtonPressed);//开火按钮按下触发函数
	PlayerInputComponent->BindAction("TabList", IE_Released, this, &ABlasterCharacter::TabButtonReleased);//开火按钮松开触发函数
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ABlasterCharacter::ReloadButtonPressed);//按R重装弹夹

	PlayerInputComponent->BindAction("SwapPrimaryWeapon", IE_Pressed, this, &ABlasterCharacter::SwapPrimaryWeapon);//按1换主武器
	PlayerInputComponent->BindAction("SwapSecondaryWeapon", IE_Pressed, this, &ABlasterCharacter::SwapSecondaryWeapon);//按2换副武器
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &ABlasterCharacter::DropCurrentWeapon);//按G丢弃当前装备的武器
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);//按4掏手雷

}


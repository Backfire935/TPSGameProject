// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include <string>
#include "GameFramework/SpringArmComponent.h"//���ɱ�����Ŀ��ļ�
#include "Camera/CameraComponent.h"	//���������Ŀ��ļ�
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

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;//��ɫ��������:�����ܵĵ���λ�ã�����������ײҲ��Ȼ����

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //�������ɱ۲�����ΪCameraBoom
	CameraBoom->SetupAttachment(GetMesh());	//���ɱ������������������
	CameraBoom->TargetArmLength = 600.0f;		//���ɱ�Ŀ��۳�����Ϊ600
	CameraBoom->bUsePawnControlRotation = true;   //ʹ��pawn������ת

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false; //��ɫ������������򣬼����
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);//����combat�����縴��

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//��ɫ�ƶ�������ý�ɫĬ���ܹ��¶ף���ͼ���ڽ�ɫ��ͼ��character movement�����crouch����
	GetCharacterMovement()->RotationRate =FRotator(0.f,0.f,850.f);//���ý�ɫת������

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//���ö�����������ײ����
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);//���������ײ��������Ϊ�Զ���ĺ�SkeletalMesh����
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//���ö�����������ײ����
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//���ö����߼�����ײ


	TurningInPlace = ETurningInPlace::ETIP_NotTurning;//�����ȳ�ʼ������ת����������

	NetUpdateFrequency = 66.f;//�������Ƶ�ʣ�ÿ��66��
	MinNetUpdateFrequency = 33.f;//��С�������Ƶ�ʣ�ÿ��33��

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//
	//�����hitbox
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

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const//replicate����ע��
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);//ͷ�ļ�#include "Net/UnrealNetwork.h"
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
}//�����ʼ�����

void ABlasterCharacter::PlayFireMontage(bool bAiming)//���ſ������̫�涯��
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);//���ſ������̫�涯��
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//ѡ���ǿ�������̫�滹��û������
		AnimInstance->Montage_JumpToSection(SectionName);//ֱ����ת���ƶ�����̫�涯��
	}
}

void ABlasterCharacter::PlayReloadMontage()//������װ���еĶ�����̫��
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)//�����������Ƿ�Ϊ�գ�ͬʱ������Ƿ�����Ѿ�װ��������
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//��ȡ��ɫ��ģ���ٻ�ȡ����ʵ��
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);//����װ������̫�涯��
		FName SectionName;
		
		switch (Combat->EquippedWeapon->GetWeaponType())//ѡ�����������ಢ���Ŷ���������µ��������ڴ˴���Ӵ���
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

		AnimInstance->Montage_JumpToSection(SectionName);//ֱ����ת���ƶ�����̫�涯��
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);//�����������̫�涯��
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);//���������׵���̫�涯��
	}
}

void ABlasterCharacter::PlayHitReactMontage()//���ű����к���ܻ�Ч��������̫��
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}//���鱻���е����Ƿ�������������������򲻻Ქ���ܻ�����
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
	
		AnimInstance->Montage_Play(HitReactMontage);//���ſ������̫�涯��
		FName SectionName("FromForward");
	//	SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//ѡ���ǿ�������̫�滹��û������
		AnimInstance->Montage_JumpToSection(SectionName);//ֱ����ת���ƶ�����̫�涯��
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
	if(bElimed) return;//����Լ��Ѿ����˾Ͳ������ܵ��˺���

	float DamageToHealth = Damage;
	float NotHitedShield = 0.f;
	if(Shield>0)
	{
		if(DamageToHealth > Shield)//�˺��Ȼ��ܸߣ��Ʒ���Ѫ
		{
			NotHitedShield = Shield;//�洢һ�λ�û�յ�����ʱ�Ļ���ֵ
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//�����µĻ���ֵ���ܵ����˺�
			DamageToHealth = DamageToHealth - NotHitedShield;//���ö�Ѫ����ɵ��˺�Ϊ�۳����ֵܵ�����˺�

		}
		else//�˺��Ȼ��ܵͣ�ֻ�л��ܽ���
		{
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//���û���ֵ���ܵ����˺�
			DamageToHealth = 0.f;//���ι��������Ѫ�����Ӱ��
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);//��������ֵ���ܵ����˺�
	UpdateHUDShield();
	UpdateHUDHealth();

	PlayHitReactMontage();
	if (Health == 0.f)//���Ѫ����0,������̭
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

void ABlasterCharacter::OnRep_ReplicatedMovement()//����ɫת��Ӧ�õ�ģ�����
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();//ģ�����ĳ���
	TimeSinceLastMovementReplication = 0.f;//��ʱ���ʱ������Ϊ0
	
}

void ABlasterCharacter::Elim()//ֻ���ڷ������ϵ���
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


void ABlasterCharacter::MulticastElim_Implementation()//�����̭ʱ
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);//����������HUD�ӵ�����Ϊ0
	}
	bElimed = true;
	PlayElimMontage();

	//��ʼ�ܽ�Ч��
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//���ý�ɫ�ƶ�
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	/*if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}�����������*/
	bDisableGameplay = true;//ֱ�ӽ���������Ϊtrue�������������������Ӧ
	GetCharacterMovement()->DisableMovement();//��ɱ���֮��������ƶ���������ֹ��ҵ������
	if(Combat)
	{
		Combat->FireButtonPressed(false);//���������ʱ�����������ڰ�������͹ص�
	}
	//������ײ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//������̭��Ч
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
	{//�����ɵ�λ�ò�����̭��Ч
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

void ABlasterCharacter::ElimTimerFinished()//��̭��ʱ������֮��,����Ϸģʽ�Ǹ������
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);//���󸴻����
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

	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)//ֻ�������û���ת���ؿ���ʱ�����������
	{
		Combat->EquippedWeapon->Destroyed();
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();//���ɳ�ʼ�����������İ���Ҫ����ͼ������
	UpdateHUDAmmo();//�õ���ʼ����������HUD��ҩ
	UpdateHUDHealth();//��������ֵHUD
	UpdateHUDShield();//���»���HUD
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	if(AttachedGrenade)//��ʼ���������׵Ŀ�����Ϊfasle
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);//���ƽ�ɫת��ĺ���

	HideCameraIfCharacterClose();//�����ɫ������ľ�����������ؽ�ɫ��ǹе��ģ��
	PollInit();//��ʼ��BlasterPlayerState

}

void ABlasterCharacter::RotateInPlace(float DeltaTime)//���ƽ�ɫת��ĺ���
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())//Enum����,ROLE_NoneֵΪ0,ROLE_SimulatedProxyֵΪ1,ROLE_AutonomousProxyΪ2,��������
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;//��ʱ��
		if (TimeSinceLastMovementReplication > 0.1f)//�ﵽ0.1s�͵���
		{
			OnRep_ReplicatedMovement();//����ɫת��Ӧ�õ�ģ�����
		}
		CalculateAO_Pitch();//����Χ��pitch����ת
	}
}
void ABlasterCharacter::MoveForward(float Value)//��ǰ�ƶ�
{

	if(bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,  Controller->GetControlRotation().Yaw, 0.f); //����
		const FVector   Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));//X����
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)//�����ƶ�
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); //����
		const FVector   Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));//Y����
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)//���Yawת��
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)//���Pitchת��
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()//E������Ӧ
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (Combat)
	{
		if (HasAuthority())//����ڷ�������
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else//�����ڿͻ����ϴ����ˣ������RPC 
		{
			ServerEquipButtonPressed();
		}
	}

}

void ABlasterCharacter::CrouchButtonPressed()//�¶׵���Ӧ
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (bIsCrouched)
		UnCrouch();
	else
	Crouch();
}

void ABlasterCharacter::AimButtonPressed()//����Ҽ����µ���Ӧ
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if(Combat)
	{ 
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()//����Ҽ��ɿ�����Ӧ
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

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

void ABlasterCharacter::DropCurrentWeapon()//��G������
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->SecondaryWeapon)//����֮����������еڶ����������Զ��л����ڶ�������
		{
			Combat->EquippedWeapon->Dropped();//�Ȱ����ϵĶ���
			Combat->EquippedWeapon = Combat->SecondaryWeapon;//��Ҫװ������������Ϊ�ڶ�������
			UE_LOG(LogTemp, Warning, TEXT("test"));
			//����Ҫʹ�õ�������״̬
			Combat->EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//����������״̬Ϊ��װ��
			Combat->AttachActorToRightHand(Combat->EquippedWeapon);//������������
			Combat->EquippedWeapon->SetOwner(this);//��������Ȩ
			Combat->EquippedWeapon->SetHUDAmmo();//���õ�ǰ��ҩHUD
			Combat->UpdateCarriedAmmo();//����Я���ĵ�ҩ
			Combat->PlayEquipWeaponSound(Combat->EquippedWeapon);//���ż�������������
			Combat->ReloadEmptyWeapon();//����Ҫ�ǿյľ�װ�ӵ�

			//��û���õĵڶ���������ָ����Ϊ�գ����ھ�ֻ��һ��������
			Combat->SecondaryWeapon = nullptr;//��Ϊģ�ͻ��ڣ����ʱ�����ٺ��ϵ�ģ��,���ʱ���ɫֻ��������û�и�����
		}//���û�еĻ����������״̬�л�����
		else//������ֻ���������Ļ�
		{
			Combat->EquippedWeapon->Dropped();//ǹ���˾���
			Combat->EquippedWeapon = nullptr;//ָ���ÿ�
			GetCharacterMovement()->bOrientRotationToMovement = true;//������ɫ���ƶ��ķ�����ת
			bUseControllerRotationYaw = false; //�رս�ɫ��������������ת
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

	return Velocity.Size(); //��ȡ��ά������ģ��������ZΪ0
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;//�������û����ֱ�ӷ��ؿ�



	float Speed = CalculateSpeed(); //��ȡ��ά������ģ��������ZΪ0

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)//���ڿ��в��ڶ�
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//ÿ֡�洢Yaw
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		//bUseControllerRotationYaw = false;//�رս�ɫ������ת

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;//�رս�ɫ������ת

		TurnInPlace(DeltaTime);//���н�ɫ����ĳ�������������61
	}

	if (Speed > 0.f || bIsInAir)//�ܲ���������ʱ��
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//ÿ֡�洢Yaw
		AO_Yaw = 0.f;	
		bUseControllerRotationYaw = true;//������ɫ������ת
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();//����Χ��pitch����ת

}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90 && !IsLocallyControlled())//�����Ŀͻ��˷�����AO_Pitch��ֵת���������⣬�������58
	{
		//��[270,360)ӳ�䵽[-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);

	}
}

void ABlasterCharacter::SimProxiesTurn()//ģ�����ĳ���
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
	ProxyYaw =  UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;//������һ֡�����ڵ���ת��ֵ
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)//�޸�ģ������ɫ�ĳ���
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

void ABlasterCharacter::Jump()//��д��Ծ������Ƕ׷��ģ����ո�Ҳ�ɻָ�վ��
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{
		Super::Jump();//ִ�и����Jump����
	}
}

void ABlasterCharacter::FireButtonPressed()//���¿����
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()//�ɿ������
{
	if (bDisableGameplay)return;//�������Ϊtrue�����������˴�������

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.F)//������ת�Ƕȳ���90��
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}

	else if (AO_Yaw < -90.F)//������ת�Ƕȳ���90��
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);//��ֵ����
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)//��ת��������Ļ�
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//ÿ֡�洢Yaw

		}
	}
		
	
}
	
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)//�ͻ��˵�����ʰȡ��ʾ����Rep_Notify
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

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()//����˵�������ײ������Ӧ
{
	if (Combat)//�������������ײ���ӷ�Χ�ڣ���ʰȡ
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}

}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)//����ɫ���������ľ������ʱ���ý�ɫ�ڱ�����ʧ�Ӷ���Ӱ����Ұ
	{
		GetMesh()->SetVisibility(false);//�������ﲻ�ɼ�	
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//����ӵ���߲��ɼ�
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);//��������ɼ�	
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//����ӵ���߿ɼ�
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(Health < LastHealth)//���ڵ�Ѫ����֮ǰ�ĵͣ�˵���ܵ����˺�
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)//���ڵĻ��ܱ�֮ǰ�ĵͣ�˵���ܵ����˺�
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

void ABlasterCharacter::StartDissolve()//��ʼ�ܽ�
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)//����ʰȡ�������ʾ
{
	if (OverlappingWeapon)//����Ѿ������������͹ص�ʰȡ��ʾ
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;//����������������

	if (IsLocallyControlled())//��������Ȩ�ڱ��ص�����£���δ��������ʰȡ�Ļ�
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);//������ʰȡ����
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()//����Ƿ�װ������
{
	return (Combat && Combat->EquippedWeapon);//Combat����Ϊ����EquippedWeaponΪtrue���ܷ���true
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



	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);//E��
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);//Ctrl��
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);//����Ҽ����´�������
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);//����Ҽ��ɿ���������
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);//����ť���´�������
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);//����ť�ɿ���������

	PlayerInputComponent->BindAction("TabList", IE_Pressed, this, &ABlasterCharacter::TabButtonPressed);//����ť���´�������
	PlayerInputComponent->BindAction("TabList", IE_Released, this, &ABlasterCharacter::TabButtonReleased);//����ť�ɿ���������
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &ABlasterCharacter::ReloadButtonPressed);//��R��װ����

	PlayerInputComponent->BindAction("SwapPrimaryWeapon", IE_Pressed, this, &ABlasterCharacter::SwapPrimaryWeapon);//��1��������
	PlayerInputComponent->BindAction("SwapSecondaryWeapon", IE_Pressed, this, &ABlasterCharacter::SwapSecondaryWeapon);//��2��������
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &ABlasterCharacter::DropCurrentWeapon);//��G������ǰװ��������
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);//��4������

}


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimeLineComponent.h"
#include"Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;//玩家输入控件
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	virtual void OnRep_ReplicatedMovement() override;//将角色转向应用到模拟代理的回调

	void Elim(bool bPlayerLeftGame);//这个只在server上调用
	void DropOrDestroyWeapon(class AWeapon* Weapon);
	void DropOrDestroyWeapons();
	//销毁武器的问题

	void SetSpawnPoint();//设置出生点
	void OnPlayerStateInitialized();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);//玩家淘汰时

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;//用于在游戏将结束时禁用,移动，开火，瞄准，换弹，装备武器，跳跃，下蹲

	UFUNCTION(BlueprintImplementableEvent)//可在蓝图或关卡蓝图中实现的函数
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();//更新生命值
	void UpdateHUDShield();//更新护盾

	void UpdateHUDAmmo();//更新弹药HUD

	void SpawnDefaultWeapon();

	UPROPERTY()
		TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
		void ServerLeaveGame();//客户端RPC请求服务端离开游戏

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

protected:

	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void EquipButtonPressed(); //E键
	void CrouchButtonPressed(); //Ctrl键
	void AimButtonPressed();//鼠标右键瞄准
	void AimButtonReleased();//鼠标右键松开解除瞄准
	void ReloadButtonPressed();//按R重装弹夹
	void SwapPrimaryWeapon();//按1切换到主武器
	void SwapSecondaryWeapon();//按2切换到副武器
	void GrenadeButtonPressed();//按4掏手雷
	void DropCurrentWeapon();//按G丢弃当前武器

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	//获取Yaw的offset
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void TabButtonPressed();//查看战绩表
	void TabButtonReleased();//关闭战绩表

	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType * DamageType, class AController* InstigatorController, AActor * DamageCauser);

	void PollInit();//初始化相关类

	void RotateInPlace(float DeltaTime);//控制角色转向的函数

	//
	//身体的hitbox
	//
	UPROPERTY(EditAnywhere)
		class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
		UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_r;



private:
	UPROPERTY(VisibleAnywhere, Category = camera)
	class USpringArmComponent* CameraBoom;	

	UPROPERTY(VisibleAnywhere, Category = camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	//角色组件类

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComp;

	UFUNCTION(Server, Reliable)//RPC 呼叫主机调用函数
	void ServerEquipButtonPressed();

	float AO_Yaw;
		float AO_Pitch;

		float	InterpAO_Yaw;
		FRotator StartingAimRotation;

		ETurningInPlace TurningInPlace;//转向的枚举类型

		void TurnInPlace(float DeltaTime);//转向函数

		//几种蒙太奇动画
		UPROPERTY(EditAnywhere, category = Combat )
		class UAnimMontage* FireWeaponMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			 UAnimMontage* HitReactMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			 UAnimMontage* ElimMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			UAnimMontage* ReloadMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			UAnimMontage* ThrowGrenadeMontage;

		void HideCameraIfCharacterClose();

		UPROPERTY(EditAnywhere, category = Combat)
		float CameraThreshold = 200.f;//相机阈值

		bool bRotateRootBone;

		float TurnThreshold = 3.5f;//达到转向要求的差值角度

		FRotator ProxyRotationLastFrame;//上一帧的代理旋转
		FRotator ProxyRotation;

		float ProxyYaw;

		float TimeSinceLastMovementReplication;

		float CalculateSpeed();

	//生命值
		UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Health,  EditAnywhere, Category = "Player Stats")
		float Health = 100.f;

		UFUNCTION()
		void OnRep_Health(float LastHealth);

	//护盾
		UPROPERTY(EditAnywhere, Category = "Player Stats")
			float MaxShield = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
			float Shield = 100.f;


		UFUNCTION()
			void OnRep_Shield(float LastShield);


		class	 ABlasterPlayerController* BlasterPlayerController;

		bool bElimed = false;

		FTimerHandle ElimTimer;

		UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;

		void ElimTimerFinished();

		bool bLeftGame = false;

		UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

		UPROPERTY(EditAnywhere)
			UCurveFloat* DissolveCurve;

		//溶解效果
		FOnTimelineFloat DissolveTrack;//溶解效果的时间轴

		UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
		void StartDissolve();

		//在运行时可以改变的动态材质实例
		UPROPERTY(VisibleAnywhere, category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;//动态溶解材质实例

		//在蓝图中设置的动态材质实例
		UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;

		//队伍颜色
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* OriginalMaterial;

		//淘汰效果
		UPROPERTY(EditAnywhere)
		UParticleSystem* ElimBotEffect;

		UPROPERTY(VisibleAnywhere)
		UParticleSystemComponent* ElimBotComponent;

		UPROPERTY(EditAnywhere)
		class USoundCue* ElimBotSound;

		class ABlasterPlayerState *BlasterPlayerState;

		UPROPERTY(EditAnywhere)
		class UNiagaraSystem* CrownSystem;

		UPROPERTY()
		class UNiagaraComponent* CrownComponent;

	//投掷物
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere,Category= "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	//是否开启友军伤害
	UPROPERTY(EditAnywhere, Category = "Team")
	bool bTeamDamage =false;

	//友军伤害倍率
	UPROPERTY(EditAnywhere, Category = "Team")
		float TeamDamageRate = 1.f;

public:	

	void  SetOverlappingWeapon(AWeapon* Weapon);//若是服务器本机拾取到了
	bool IsWeaponEquipped();//用于和BlasterAnimInstance蓝图通信的函数，返回是否装备武器的信息
	bool IsAiming();//用于和BlasterAnimInstance蓝图通信的函数，返回是否正在瞄准

	AWeapon *GetEquippedWeapon();

	FVector GetHitTarget() const;

	ECombatState GetCombatState() const;

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	FORCEINLINE bool IsElimed() const { return bElimed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE void SetHealth(float Amount)  { Health = Amount; }

	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }

	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGamePlay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComp; }
	FORCEINLINE bool IsHoldingTheFlag() const;

	bool IsLocallyReloading();

	ETeam GetTeam();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimeLineComponent.h"
#include"Blaster/BlasterTypes/CombatState.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;//�������ؼ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	virtual void OnRep_ReplicatedMovement() override;//����ɫת��Ӧ�õ�ģ�����Ļص�

	void Elim();//���ֻ��server�ϵ���
	void DropOrDestroyWeapon(class AWeapon* Weapon);
	void DropOrDestroyWeapons();
	//��������������



	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();//�����̭ʱ

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;//��������Ϸ������ʱ����,�ƶ���������׼��������װ����������Ծ���¶�

	UFUNCTION(BlueprintImplementableEvent)//������ͼ��ؿ���ͼ��ʵ�ֵĺ���
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();//��������ֵ
	void UpdateHUDShield();//���»���

	void UpdateHUDAmmo();//���µ�ҩHUD

	void SpawnDefaultWeapon();

	UPROPERTY()
		TMap<FName, class UBoxComponent*> HitCollisionBoxes;
protected:

	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void EquipButtonPressed(); //E��
	void CrouchButtonPressed(); //Ctrl��
	void AimButtonPressed();//����Ҽ���׼
	void AimButtonReleased();//����Ҽ��ɿ������׼
	void ReloadButtonPressed();//��R��װ����
	void SwapPrimaryWeapon();//��1�л���������
	void SwapSecondaryWeapon();//��2�л���������
	void GrenadeButtonPressed();//��4������
	void DropCurrentWeapon();//��G������ǰ����

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	//��ȡYaw��offset
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void TabButtonPressed();//�鿴ս����
	void TabButtonReleased();//�ر�ս����

	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType * DamageType, class AController* InstigatorController, AActor * DamageCauser);

	void PollInit();//��ʼ�������

	void RotateInPlace(float DeltaTime);//���ƽ�ɫת��ĺ���

	//
	//�����hitbox
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

	//��ɫ�����

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

	UFUNCTION(Server, Reliable)//RPC �����������ú���
	void ServerEquipButtonPressed();

	float AO_Yaw;
		float AO_Pitch;

		float	InterpAO_Yaw;
		FRotator StartingAimRotation;

		ETurningInPlace TurningInPlace;//ת���ö������

		void TurnInPlace(float DeltaTime);//ת����

		//������̫�涯��
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
		float CameraThreshold = 200.f;//�����ֵ

		bool bRotateRootBone;

		float TurnThreshold = 3.5f;//�ﵽת��Ҫ��Ĳ�ֵ�Ƕ�

		FRotator ProxyRotationLastFrame;//��һ֡�Ĵ�����ת
		FRotator ProxyRotation;

		float ProxyYaw;

		float TimeSinceLastMovementReplication;

		float CalculateSpeed();

	//����ֵ
		UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Health,  EditAnywhere, Category = "Player Stats")
		float Health = 100.f;

		UFUNCTION()
		void OnRep_Health(float LastHealth);

	//����
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

		UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

		UPROPERTY(EditAnywhere)
			UCurveFloat* DissolveCurve;

		//�ܽ�Ч��
		FOnTimelineFloat DissolveTrack;//�ܽ�Ч����ʱ����

		UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
		void StartDissolve();

		//������ʱ���Ըı�Ķ�̬����ʵ��
		UPROPERTY(VisibleAnywhere, category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;//��̬�ܽ����ʵ��

		//����ͼ�����õĶ�̬����ʵ��
		UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;

		//��̭Ч��
		UPROPERTY(EditAnywhere)
		UParticleSystem* ElimBotEffect;

		UPROPERTY(VisibleAnywhere)
		UParticleSystemComponent* ElimBotComponent;

		UPROPERTY(EditAnywhere)
		class USoundCue* ElimBotSound;

		class ABlasterPlayerState *BlasterPlayerState;

	//Ͷ����
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere,Category= "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

public:	

	void  SetOverlappingWeapon(AWeapon* Weapon);//���Ƿ���������ʰȡ����
	bool IsWeaponEquipped();//���ں�BlasterAnimInstance��ͼͨ�ŵĺ����������Ƿ�װ����������Ϣ
	bool IsAiming();//���ں�BlasterAnimInstance��ͼͨ�ŵĺ����������Ƿ�������׼

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
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensation; }


	bool IsLocallyReloading();
	
};

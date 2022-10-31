// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include"Blaster/HUD/BlasterHUD.h"
#include"Blaster/Weapon/WeaponTypes.h"
#include"Blaster/BlasterTypes/CombatState.h"
#include "Blaster/Weapon/Weapon.h"

#include "CombatComponent.generated.h"

#define TraceLength 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();

	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(class AWeapon* WeaponToEquip);//װ������

	void SwapPrimaryWeapon();//�л���������
	void SwapSecondaryWeapon();//�л���������


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//ʹ���������縴�ƣ���Ҫʹ�ô˺�������ע��

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();

	void JumpToShotGunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LauncherGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLuncherGrenade(const FVector_NetQuantize& Target);

	void PickupAmmp(EWeaponType WeaponType,int32 AmmoAmount);
protected:

	virtual void BeginPlay() override;

	void SetAiming(bool IsAiming);//�����Ƿ���׼

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);//ServerSetAiming_Implementionʵ�ֵ������Ƿ���׼��Serverͬ��

	UFUNCTION()
	void OnRep_EquippedWeapon();//װ��������Rep_Notify

	UFUNCTION()
		void OnRep_SecondaryWeapon();//װ���ڶ���������Rep_Notify

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);//����������

	UFUNCTION(NetMulticast,  Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);//�������������ͻ��˵Ķಥ����

	void TraceUnderCrosshairs(FHitResult & TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor * ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);

	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound(AWeapon * WeaponToEquip);

	void ReloadEmptyWeapon();

	void SelectWeaponSocket(FName SocketName);

	UFUNCTION(BlueprintCallable)
		void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION()
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

private:
	UPROPERTY()
	 class ABlasterCharacter* Character;

	UPROPERTY()
	 class ABlasterPlayerController* Controller;

	UPROPERTY()
	 class ABlasterHUD* HUD;

	 UPROPERTY( ReplicatedUsing = OnRep_EquippedWeapon)//ʹ���������縴�ƣ���Ҫʹ��GetLifetimeReplicatedProps����ע��
	  AWeapon* EquippedWeapon;

	 UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	 AWeapon* SecondaryWeapon;
	
	 UPROPERTY(Replicated)//ʹ���������縴�ƣ���Ҫʹ��GetLifetimeReplicatedProps����ע��
	 bool bAiming;//�����Ƿ�Ҫ��׼

	 UPROPERTY(EditAnywhere)
	 float BaseWalkSpeed;//���������ٶ�

	 UPROPERTY(EditAnywhere)
	 float AimWalkSpeed;//��׼�����ٶ�
	 
	 UPROPERTY(EditAnywhere)
	 bool bFireButtonPressed;

	//HUD��׼��
	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairVelocityFactor;//Ĭ�ϵ������ٶ�׼����ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairInAirFactor;//Ĭ�ϵ�׼���ڿ��е���ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimFactor;//Ĭ�ϵ�׼����׼��ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimTarget = 0.58f;//Ĭ�ϵ���׼״̬��׼�ǵ���ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
		 float CrosshairShootingFactor;//Ĭ�ϵ���׼״̬��׼�ǵ���ɢ

	 FVector HitTarget;

	 FHUDPackage HUDPackage;

	 //��׼ʱ��FOV
	 UPROPERTY( EditAnywhere,Category = Combat)
		 float DefaultFOV;

	 UPROPERTY(EditAnywhere , Category = Combat)
	 float ZoomedFOV = 30.f;

	 float CurrentFOV;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float ZoomInterpSpeed = 20.f;

	 void InterpFOV(float DeltaTime);

	 //ȫ�Զ����
	 FTimerHandle FireTimer;

	 bool bCanFire = true;

	 void StartFireTimer();
	 void FireTimerFinished();
	 void Fire();

	 bool CanFire();

	 //��ǰ�ֳ������ı���
	 UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	 int32 CarriedAmmo;//����

	 UFUNCTION()
		 void OnRep_CarriedAmmo();

	 TMap<EWeaponType, int32> CarriedAmmoMap;//Map���ͣ�hash�㷨�޷���local��server�ϻ����ͬ�Ľ���������м�����������縴�ƶ�����TMap���ͱ���

	 void InitializeCarriedAmmo();

	 UPROPERTY(EditAnywhere)
	 int32	StartingARAmmo = 30;//��ǹ�ӵ�

	 UPROPERTY(EditAnywhere)
		 int32	StartingRocketAmmo = 0;//���Ͳ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingPistolAmmo = 0;//��ǹ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingSMGAmmo = 0;//��ǹ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingShotGunAmmo = 0;//���ӵ�ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingSniperAmmo = 0;//���ӵ�ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingGrenadeLauncherAmmo = 0;//���ӵ�ҩ


	 UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	 ECombatState CombatState = ECombatState::ECS_Unoccupied;//������״̬��Ĭ���ǿ���״̬

	 UFUNCTION()
	 void OnRep_CombatState();

	 void UpdateAmmoValues();

	 void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_Grenades,EditAnywhere)
	 int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	 int32 MaxGrenades = 9;

	void UpdateHUDGrenades();
public:	
	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
};

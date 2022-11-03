// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Weapon_Initial UMETA(DisplayName = "Initial State"),
	Weapon_Equipped UMETA(DisplayName = "Equipped"),
	Weapon_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	Weapon_Dropped UMETA(DisplayName = "Dropped"),
	Weapon_MAX UMETA(DisplayName = "DefaultMax")

};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMax")

};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	//�����������縴�Ƶķ���
	//.Cpp�ļ��У�GetLifetimeReplicatedProps���������DORELIFETIME(�����ƣ�������)
	//����ǰ���UPROPERTY(Replicated)

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();

	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector &HitTarget);
	void Dropped();
	FVector TraceWithScatter(const FVector& HitTarget);//����ɢ������߼��

	UFUNCTION()
		void ReadyDestroyWeapon();
	UFUNCTION()
		void DestroyWeapon();
	FTimerHandle DestroyWeaponTimer;
	//bool bIsDroppedToDestroyed = false;//�����Ƿ񱻶������ϵȴ�����
	UPROPERTY(EditAnywhere, Category = "Combat")
		float DestroyWeaponTime;

	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, category = Combat)
		float FireDelay = 0.15f;//�Զ�����Ĭ�Ͽ�����

	UPROPERTY(EditAnywhere, category = Combat)
		bool bAutoMatic = true;//Ĭ����������ģʽ:ȫ�Զ�
	/*
����׼�ŵ�����
*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsBottom;

	//��׼ʱ��ҰFOV�仯
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.F;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;//װ������ʱ����Ч

	//�����������Զ�����Ⱦ���
	void EnableCustomDepth(bool bEnable);

	bool bDestroyedWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseScatter = false;//�Ƿ������ӵ�ɢ��
	 
	protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();

	
	virtual void HandleWeaponEquiped();//����װ����������
	virtual void HandleWeaponDropped();//������������
	virtual void HandleWeaponSecondary();//����ڶ�������

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor * OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	//���߽�����λ�÷����ӵ�
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;//���ӵ���Ч��������

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;//��ɢ��Χ
private:
	UPROPERTY(VisibleAnywhere, category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	//���һ����������ΪΪRep_Notify,���ñ�����������ʱ���յ���ֵ�Ŀͻ��ˣ������Ե���һ���Զ���ĺ�����
	//�÷�Ϊ����UPROPERTY����ReplicatedUsing=xx������
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, category = "Weapon Properties")
		EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;	

	UPROPERTY(EditAnywhere)
	TSubclassOf<class  ABulletShells> BulletShells;

	
	UPROPERTY(EditAnywhere)
	int32 Ammo;//�����������ӵ�����

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;//��������

	int32 Sequence = 0;//���ӵ������йص�δ���������������������	

	UFUNCTION()
	void SpendRound();//�����˸������ӵ�

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;



public:	

	 void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }

	bool IsEmpty();//�ж��ӵ��Ƿ����
	bool IsFull();//�ж��ӵ��Ƿ�������
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; };
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

};

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步
	//设置属性网络复制的方法
	//.Cpp文件中，GetLifetimeReplicatedProps函数中添加DORELIFETIME(类名称，变量名)
	//属性前添加UPROPERTY(Replicated)

	virtual void OnRep_Owner() override;

	void SetHUDAmmo();

	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector &HitTarget);
	void Dropped();
	FVector TraceWithScatter(const FVector& HitTarget);//喷子散射的射线检测

	UFUNCTION()
		void ReadyDestroyWeapon();
	UFUNCTION()
		void DestroyWeapon();
	FTimerHandle DestroyWeaponTimer;
	//bool bIsDroppedToDestroyed = false;//武器是否被丢到地上等待销毁
	UPROPERTY(EditAnywhere, Category = "Combat")
		float DestroyWeaponTime;

	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, category = Combat)
		float FireDelay = 0.15f;//自动武器默认开火间隔

	UPROPERTY(EditAnywhere, category = Combat)
		bool bAutoMatic = true;//默认武器开火模式:全自动
	/*
武器准信的纹理
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

	//瞄准时视野FOV变化
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.F;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;//装备武器时的音效

	//开关武器的自定义渲染深度
	void EnableCustomDepth(bool bEnable);

	bool bDestroyedWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseScatter = false;//是否允许子弹散射
	 
	protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();

	
	virtual void HandleWeaponEquiped();//处理装备过的武器
	virtual void HandleWeaponDropped();//处理丢弃的武器
	virtual void HandleWeaponSecondary();//处理第二把武器

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

	//射线结束的位置分裂子弹
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;//喷子的有效攻击距离

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;//扩散范围
private:
	UPROPERTY(VisibleAnywhere, category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	//如果一个变量被设为为Rep_Notify,当该变量发生复制时，收到该值的客户端，都可以调用一个自定义的函数。
	//用法为，在UPROPERTY设置ReplicatedUsing=xx函数名
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
	int32 Ammo;//满弹夹数量子弹数量

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;//备弹数量

	int32 Sequence = 0;//跟子弹数量有关的未被服务器处理的请求数量	

	UFUNCTION()
	void SpendRound();//打完了更新下子弹

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

	bool IsEmpty();//判断子弹是否打完
	bool IsFull();//判断子弹是否是满的
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; };
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��

	void SetHUDHealth(float Health, float MaxHealth);//��������ֵ

	void SetHUDShield(float Shield, float MaxShield);//��������ֵ

	void SetHUDScore(float Score);//���û�ɱ��,������float����ΪSetScore()Ĭ���õ�float

	void SetHUDDefeats(int32 Defeats);//������������

	void SetHUDWeaponAmmo(int32 Ammo);//���õ�ǰ���е�ҩ

	void SetHUDCarriedAmmo(int32 Ammo);//���õ�ǰ���е�ҩ

	void SetHUDMatchCountDown(float CountdownTime);//���öԾ��ڿ�ʼʱ��

	void SetHUDAnnouncementCountdown(float CountdownTime);//������Ϸ��������ʱ��

	void SetHUDGrenades(int32 Grenades);//�������׵�����

	virtual void OnPossess(APawn* InPawn) override; 

	void SetElimText(bool bElim);//������̭ʱ��HUD��ʾ
	
	virtual  void Tick(float DeltaTime) override;

	virtual void ReceivedPlayer() override;//�ڽ�����ҵ�ʱ���ִ��GetServerTime��Ŀ���Ǿ���������ʱ�䱣��һ��

	virtual float GetServerTime();//�������ʱ��ͬ��
	/*
	*����˺Ϳͻ���֮����첽ʱ�����
	*/
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);//������ǿͻ��˷�������ʱ��ʱ�䣬Ҫ���ȡ����˵�ʱ��

	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);//������˵�ǰ��ʱ�䷢�͸��ͻ��ˣ�����Ӧ�ͻ��˷����ServerRequestServerTime����

	float ClientServerDelta = 0.f;//�ͻ��˺ͷ���˵�ʱ�����

	UPROPERTY(EditAnywhere,Category = Time)
	float TimeSyncFrequency = 5.f;//ÿ��һ��ʱ���Զ����һ�����������ʱ�Ĭ����5s

	float TimeSyncRunningTime = 0.f;//������һ�μ���ʱ��

	void CheckTimeSync(float DeltaTime);//���ʱ��ͬ���ĺ���

	void OnMatchStateSet(FName State);

	void HandleMatchHasStarted();

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();//����˼����Ϸ״̬

		UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown,float StartingTime);//�ͻ�����;������Ϸʱͬ������

		void HandleCooldown();

		void HighPingWarning(float ping);
		void StopHighPingWarning();
		void CheckPing(float DeltaTime);

protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	void PollInit();//���ڼ��CharacterOverlay�û��ؼ��Ƿ񴴽���������������Ӧ�����е�����ֵ

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	float LevelStartingTime;//��Ϸ��ʼ��ʱ��

	float MatchTime = 0.f;//

	float WarmupTime = 0.f;//����ʱ��

	uint32 CountdownInt = 0;

	float CooldownTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;//����׷����Ϸ��״̬

	UFUNCTION()
	void OnRep_MatchState();//�ÿͻ���Ҳ֪����Ϸ��״̬

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;


	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 10.f;//���ʮ����һ��pingֵ

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f; //��ping�������ֵ,100���¶�����
};

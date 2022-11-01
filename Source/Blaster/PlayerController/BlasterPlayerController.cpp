// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include"Blaster/HUD/BlasterHUD.h"
#include"Blaster/HUD/Announcement.h"
#include"Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include"Blaster/Character/BlasterCharacter.h"
#include"Net/UnrealNetwork.h"
#include"Blaster/GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include"Blaster/BlasterComponents/CombatComponent.h"
#include"Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)//�ɿͻ��˷���ģ�������ǿͻ��˷�������ʱ��ʱ�䣬Ҫ���ȡ����˵�ʱ��
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//����˽��յ��ͻ�������ʱ���ʱ��
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)//�ɷ���˷���ģ�������˵�ǰ��ʱ�䷢�͸��ͻ��ˣ�����Ӧ�ͻ��˷����ServerRequestServerTime����
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//������˵ĵ�ǰʱ���ȥ�ͻ��˷�������ʱ���ʱ�䣬�õ����紫����Ϣ���ӳ�ʱ�䡣
	float CurrentServerTime = TimeServerReceivedClientRequest +0.5 * RoundTripTime;//����˽��յ��ͻ�������ʱ���ʱ�� + 0.5*���紫����Ϣ���ӳ�ʱ�� = ��ͻ���ͬ���ķ���˵�����ʱ��
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//�ͻ��˺ͷ���˵�ʱ�����

}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = 	Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
	
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime); //���ʱ��ͬ���ĺ���
	PollInit();//���HUD���Ƿ񴴽����õ���ֵ
	CheckPing(DeltaTime);

}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			float Ping = 0;
			Ping = PlayerState->GetPingInMilliseconds();
			if (Ping > HighPingThreshold)//�õ�pingֵ���;������ֵ���бȽ�
			{
				HighPingWarning(Ping);
				PingAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0;//��ʱ������
	}
	bool bHighPingAnimationPlaying =
		BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->HighPingAnimation
		&& BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)//����ڲ��Ÿ�ping��������ֹͣ����
	{
		PingAnimationRunningTime += DeltaTime;//��������ʱ��ļ�ʱ
		if (PingAnimationRunningTime > HighPingDuration)//�������õ�ʱ��
		{
			StopHighPingWarning();//ֹͣ����
		}
	}
}

void ABlasterPlayerController::SetHUDTime()//��������HUD,�˺�������Tick�е���
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)  TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown)TimeLeft =CooldownTime+ WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	//uint32 SecondsLeft = FMath::CeilToInt( MatchTime - GetServerTime());//��Ϸ��ʱ��-��Ϸ��ʼ��ʱ��=��Ϸʣ�µ�ʱ�䣬CeilToInt��������Ȼ�����������ͱ�����ÿ֡�����������ת����uInt��ֻ�з��������ı仯ʱ�������if�жϲŻ���Ч
	uint32 SecondsLeft = FMath::CeilToInt( TimeLeft);


	/*	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}*/
	

	if (CountdownInt != SecondsLeft)//ÿ֡������HUD���������������ж�SecondsLeft�Ƿ����仯�������˱仯�Ÿı�HUD
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);//��û��ʼ���������Ѿ�����������չʾ��ʾ����
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);//�Ѿ���ʼ��Ϸ��չʾ���HUD����
		}
		//SetHUDMatchCountDown(MatchTime - GetServerTime());//ˢ��һ��HUD
	}

	CountdownInt = SecondsLeft;//���е���˵��SecondsLeftֵ�����˸ı䣬���ʱ��Ҳ������CountdownInt��ֵ
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)//���HUDû���ü��������ߴ���ʧ��
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)//tick�и���һ��
			{
				if(bInitializeHealth) SetHUDHealth(HUDHealth,HUDMaxHealth);
				if(bInitializeShield) SetHUDShield(HUDShield,HUDMaxShield);
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if(BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if(bInitializeGrenades)	SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(),BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
	}
}

void ABlasterPlayerController::SetElimText(bool bElim)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ElimText;
	if (bHUDValid)
	{
		if(bElim)
		BlasterHUD->CharacterOverlay->HealthText->SetVisibility(ESlateVisibility::Visible);
		else
		BlasterHUD->CharacterOverlay->HealthText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;//�ı�ﵽ���ͬ��Ҫ���ʱ��
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());//����ͬ��ʱ��
		TimeSyncRunningTime = 0.f;//��ʱ������
	}
}

void ABlasterPlayerController::ReceivedPlayer()//�ڽ�����ҵ�ʱ���ִ��GetServerTime��Ŀ���Ǿ���������ʱ�䱣��һ��
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

float ABlasterPlayerController::GetServerTime()//�ͷ�����ͬ��ʱ��
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else  return GetWorld()->GetTimeSeconds() + ClientServerDelta;//���ص�ǰʱ��+���������ͬ����ʱ�� = �ͷ�����ͬ����ʱ��
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar && 
		BlasterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;//����Ѫ���ٷֱ�
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));//���㲢ת����ǰѪ��ֵ���ı�
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));//����Ѫ��ֵ
	}
	else//����ؼ���ʼ��ʧ�ܣ����и�ר��������ʼ���ĺ���pollinit
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;

	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;//���û��ܰٷֱ�
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));//���㲢ת����ǰ����ֵ���ı�
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));//���û���ֵ
	}
	else//����ؼ���ʼ��ʧ�ܣ����и�ר��������ʼ���ĺ���pollinit
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;

	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else//����ؼ���ʼ��ʧ�ܣ����и�ר��������ʼ���ĺ���pollinit
	{
		bInitializeScore = true;
		HUDScore = Score;
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText)); 

	}
	else//����ؼ���ʼ��ʧ�ܣ����и�ר��������ʼ���ĺ���pollinit
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountDown(float CountdownTime)//CountdownTime��һ����Ϸ������ʱ��
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountDownText;

	if (bHUDValid)
	{

		if(CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);//��ʱ��������������60���Ƿ֣�����ȡint
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);//��ʱ��������������60���Ƿ֣�����ȡint
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
		bInitializeGrenades = true;
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime,LevelStartingTime);
	}

}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)//�����Ҳ���������׶ν�����Ϸ�Ļ����Ͳ��������������
	{
		BlasterHUD->AddAnnouncement();//��ҿ���������Ϸ�ѿ�ʼ�͵�����Ļ��ʾ����
	}
}

void ABlasterPlayerController::OnRep_MatchState()//OnMatchStateSet�����ȱ���Ϸģʽ���ú������MatchState��ֵ��Ȼ���ٴ���������������Դ�ʱMatchState�Ѿ����¹��ˣ�ֱ�Ӵ������˵�MatchStateֵ���ͻ��˼���
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();//���ÿͻ����ϵ�HandleMatchHasStart����
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();//����������ʱ���������HUD�����ʾ����,����Ѿ����˾Ͳ������
		if (BlasterHUD->Announcement)//������ʽ��Ϸ������Announcement����
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()//������Ϸ����
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();//����������ص���ɫ���HUD
		bool bHUDValid = BlasterHUD->Announcement
			&& BlasterHUD->Announcement->AnnouncementText
			&& BlasterHUD->Announcement->InfoText;
		if (bHUDValid)//������ʽ��Ϸ������Announcement����
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("�µ���ϷҪ��ʼla");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));//����ʾ��Ϣ���ı���Ϊ�µ�������Ϣ

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//��ȡ��Ϸ״̬
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();//��ȡ�Լ�����Ϸ״̬
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;//����Ϸ״̬�л�ȡ���״̬
				FString InfoTextString;
				if(TopPlayers.Num() == 0)//��������СΪ0����˵����û�����õ�����
				{
					InfoTextString = FString("�ⳡ��Ϸû��Ӯ��.");
					//InfoTextString = FString("There is no winner.");
				}
				else if(TopPlayers.Num() == 1 &&	TopPlayers[0] == BlasterPlayerState)//�����õ��˵�һ������������Լ�
				{
					InfoTextString = FString("�����ⳡ�Ծֵ÷����");
				}
				else if(TopPlayers.Num() ==1)//�����õ��˵�һ�������Լ�
				{
					InfoTextString = FString::Printf(TEXT("Ӯ����:\n%s"), *TopPlayers[0]->GetPlayerName());//��ȡ����ҵ�����
				}
				else if(TopPlayers.Num()>1)//��ֹһ�˻���˵�һ�ĵ÷�
				{
					InfoTextString = FString("�������������˶�����ʤ��:\n");
					for(auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));//�����е���ʤ�ߵ�����д��ȥ
					}
				}


				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}

		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter && BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisableGameplay = true;//����һЩ�������
		BlasterCharacter->GetCombat()->FireButtonPressed(false);//��֮ǰ������ֳ��������𣬴�ʱ��ֹͣ����
	}
}

void ABlasterPlayerController::HighPingWarning(float ping)//���Ÿ�ping���涯��
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation&&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.F);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation,
				0.f,
			5
			);
		FString Text = FString::Printf(TEXT("%.0fms"), ping);//��ping��floatתΪFString
		BlasterHUD->CharacterOverlay->PingText->SetText(FText::FromString(Text));//��ping��FStringתΪFText
	}
}


void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation&&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.F);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);//ֹͣ����
		}
		BlasterHUD->CharacterOverlay->PingText->SetText(FText());//�ÿ�
	}

}


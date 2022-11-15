// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include"Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
	
}

void ATeamsGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATeamsGameMode, TargetScore);
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//���ص�ǰ��Ϸ״̬,����������
	if (BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			//��򵥵Ļ���������ƥ��������
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//���ص�ǰ��Ϸ״̬,����������
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if(BGameState && BPState)
	{
		if(BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}

}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage , float TeamDamageRate)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;
	//�����������ͬ��������,��ֹ����
	if(AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		//�����������
		if(bTeamDamage)
		{
			//�������˺����ʵķ�Χ
			if (TeamDamageRate > 1) TeamDamageRate = 1;
			// ��Ѫ ְҵ ����
			if (TeamDamageRate < 0) TeamDamageRate = 0;
			return BaseDamage * TeamDamageRate;
		}
		else
		{
			//û�����˾ͷ������˺�
			return 0.f;
		}
	}
	return  BaseDamage;

}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//���ص�ǰ��Ϸ״̬,����������
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;//AttackerController�Ƿ���ڣ����ھ�ȡ�������״̬�������ھ����
	ABlasterPlayerState* ElimmedPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;//VictimController�Ƿ���ڣ����ھ�ȡ�������״̬�������ھ����

	if(BGameState && AttackerPlayerState && ElimmedPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ElimmedPlayerState->GetTeam())
		{//�������ɱ����ɱ���Ѳ��ӷ�,��������������
			return;
		}
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{//��̭һ���췽��Һ����Ӽ�һ��
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{//��̭һ��������Һ��Ӽ�һ��
			BGameState->RedTeamScores();
		}
		
	}
}




void ATeamsGameMode::OnRep_TargetScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDTargetScore(TargetScore);
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//���ص�ǰ��Ϸ״̬,����������
	if(BGameState)
	{
		//��ȡ������ҵ����״̬
		for(auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());//UE5�³��ĵ�TObjectָ�룬��get������ȡ
			if(BPState &&  BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				//��򵥵Ļ���������ƥ��������
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}

}

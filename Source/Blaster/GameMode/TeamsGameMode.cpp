// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include"Kismet/GameplayStatics.h"

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

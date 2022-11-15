// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include"Blaster/Character/BlasterCharacter.h"
#include"Blaster/PlayerController/BlasterPlayerController.h"
#include"Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include"Blaster/PlayerState/BlasterPlayerState.h"
#include"Blaster/GameState/BlasterGameState.h"
#include "Blaster/Weapon/Weapon.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
	
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;//�Ƴ���Ϸ��ʼ�Ľ��̣�ʹ��Ϸ״̬ΪWaitingToStart State,��Ϸģʽ��Ϊ���е��������һ��Ĭ�ϵĿ�������pawn����ҿ���ʹ�����pawn�ɱ���������,ֱ������StartMatch���������ʱ����Ϸģʽ������ָ����pawn

}

void ABlasterGameMode::Tick(float Deltatime)
{
	Super::Tick(Deltatime);
	
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//�Ӵ򿪳��򵽿�ʼ������Ϸ��ʱ��-�򿪳������ڵ�ʱ��+������ʱʱ�� = ��Ϸ��ʼ����ʱ��ʣ��ʱ��
		if (CountdownTime <= 0.f)
		{
			StartMatch();//���������������SetMatchState(MatchState::InProgress);��������Ϸ״̬
		}
	}
	else if(MatchState ==MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//����ʱ��+���õ���Ϸʱ��-��Ϸ�ӳ������е����ڵ�ʱ��+��Ϸ�ӳ������е���Ϸ��ʼ��ʱ�� = ������Ϸ������ʣ��ʱ��
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime +  WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//��Ϸ�����ʱ��+����ʱ��+���õ���Ϸʱ��-��Ϸ�ӳ������е����ڵ�ʱ��+��Ϸ�ӳ������е���Ϸ��ʼ��ʱ�� = ��Ϸ���㵹��ʱ��ʣ��ʱ��
		if(CountdownTime <=0.f)
		{
			RestartGame();//��Ϸ������ϣ�������Ϸ�Ծ�
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();//�Ӵ򿪳��򵽿�ʼ������Ϸ��ʱ��
}

void ABlasterGameMode::OnMatchStateSet()//����Ϸģʽ֪ͨ���е���ҿ��������ڵ���Ϸ״̬
{
	Super::OnMatchStateSet();
	//���Ǹ�������
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)	 //��������ѭ���������е���ҿ�����
	{
		ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState,bTeamsMatch);
		}

	}
}

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		//AWeapon* Weapon =  Cast<ABlasterCharacter>(AttackerPlayerState->GetPlayerController()->GetCharacter())->GetEquippedWeapon();

		AttackerPlayerState->AddToScore(1.f);//��һ�λ�ɱ����
		BlasterGameState->UpdateTopScore(AttackerPlayerState);//����������õ�����߷־ͻᱻ����Ϊ������ߵ���
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();//�õ���һ��������һ�ı�
			}

		}

		for(int32 i=0; i< PlayersCurrentlyInTheLead.Num(); i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				Loser->MulticastLostTheLead();//���ǵ�һ�ˣ�����Чjue��
			}
		}


	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);//��һ����������

	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayerController && AttackerPlayerState && VictimPlayerState )
		{
			BlasterPlayerController->BroadcastElim(AttackerPlayerState,VictimPlayerState);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();//�ӿ��������벢��������
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart( ElimmedController, PlayerStarts[Selection]);//����ҿ�ʼ���������
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	//������̭��Һ�����ΪbLeftGame����������ֵ
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter * CharacterLeaving =  Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());

}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage, float TeamDamageRate)
{
	return BaseDamage; 
}


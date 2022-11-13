// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include"Kismet/GameplayStatics.h"

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	if (BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			//最简单的基于人数的匹配队伍机制
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
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
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



void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	if(BGameState)
	{
		//获取所有玩家的玩家状态
		for(auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());//UE5新出的的TObject指针，用get方法获取
			if(BPState &&  BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				//最简单的基于人数的匹配队伍机制
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

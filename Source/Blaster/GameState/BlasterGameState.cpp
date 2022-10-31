// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include"Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState,TopScoringPlayers);

}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)//纪录得到分数的玩家
{
	if(TopScoringPlayers.Num() == 0 )
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();

	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);//确保输入的信息是唯一的，不会输入重复的信息

	}
	else if(ScoringPlayer->GetScore() > TopScore)//如果这个玩家的分比之前的最高分高
	{
		TopScoringPlayers.Empty();//清空之前纪录的最高分
		TopScoringPlayers.AddUnique(ScoringPlayer);//换成新的最高分
		TopScore = ScoringPlayer->GetScore();
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifeTimeProps) const override;

	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);//纪录得到分数的玩家
	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;//用于存储分数最高的玩家


private:
	float TopScore = 0.f;
};

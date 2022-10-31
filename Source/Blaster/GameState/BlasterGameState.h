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

	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);//��¼�õ����������
	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;//���ڴ洢������ߵ����


private:
	float TopScore = 0.f;
};

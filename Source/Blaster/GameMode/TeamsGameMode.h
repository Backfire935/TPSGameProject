// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage , float TeamDamageRate) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//ʹ���������縴�ƣ���Ҫʹ�ô˺�������ע��


	//��ʤ����
	UPROPERTY(ReplicatedUsing= OnRep_TargetScore,EditAnywhere)
	float TargetScore = 100.f;

	UFUNCTION()
	void OnRep_TargetScore();

	//��ȡ��ʤ�ķ���
	FORCEINLINE float GetTargetScore() const { return TargetScore; }



protected:
	virtual void HandleMatchHasStarted() override;

};

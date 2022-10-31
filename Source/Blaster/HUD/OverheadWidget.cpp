// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFrameWork/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//ENetRole LocalRole = InPawn->GetLocalRole();  //��ȡ���صĽ�ɫ
	ENetRole RemoteRole = InPawn->GetRemoteRole();  //��ȡ���ߵĽ�ɫ

	APlayerState* PlayerState =  InPawn->GetPlayerState();
	FString PlayerName = FString("");
	if (PlayerState)
	{
		PlayerName = PlayerState->GetPlayerName();
	}
	/*FString PlayerName = FString("");

	PlayerName = InPawn->GetName();
	*/
	FString Role;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority : 
		Role = FString("Authority");
		break;

	case ENetRole::ROLE_AutonomousProxy :
		Role = FString("Autonomous Proxy");
		break;

	case ENetRole::ROLE_SimulatedProxy :
		Role = FString("Simulated Proxy");
		break;

	case ENetRole::ROLE_None :
		Role = FString("None");
		break;

	default:
		break;
	}

	FString RemoteRoleString = FString::Printf(TEXT("Role :%s , Name: %s"), *Role, *PlayerName);
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);  


}

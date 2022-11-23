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
	//ENetRole LocalRole = InPawn->GetLocalRole();  //获取本地的角色
	ENetRole RemoteRole = InPawn->GetRemoteRole();  //获取在线的角色

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

//UE5.1 删除OnLevelRemoveFromWorld函数，使用NativeDestruct函数替换，5.1版本以下换回OnLevelRemoveFromWorld，感谢epic讨论区老哥救我一命
	// https://forums.unrealengine.com/t/where-is-uuserwidget-onlevelremovedfromworld-in-5-1/692215/7
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();

}

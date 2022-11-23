// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include"MultiplayerSessionSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();     //玩家数量

	UGameInstance* GameInstance = GetGameInstance();

	if(GameInstance)
	{
		//依赖于steam联机插件
		UMultiplayerSessionSubsystem * Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		//断言处理防止为空
		check(Subsystem);

		if (NumberOfPlayers >= Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true; //使用无缝地图旅行

				FString MatchType = Subsystem->DesiredMatchType;
				if(MatchType == "FreeForAll")
				{
				World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
					
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/TeamsMap?listen"));

				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/TestMap?listen"));

				}
			}
		}
	}

	
}

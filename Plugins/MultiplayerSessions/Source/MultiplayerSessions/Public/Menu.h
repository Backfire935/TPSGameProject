// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 100, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/LobbyMap")) );
protected:
	
	virtual bool Initialize() override;	//��ʼ����ť�ؼ�

	virtual void OnLevelRemovedFromWorld(ULevel *InLevel,UWorld *InWorld) override ; //���¹ؿ�ʱ�������е�ǰ�ؿ�
	// void OnLevelRemoveFromWorld(ULevel* InLevel, UWorld* InWorld) ;  //����ط��Ǵ�� Removed��дһ��d ��bug����Сʱ ��Ϊ���в�Ҫd�ĺ���

	//
	//Ϊ���˻Ự��ϵͳί�еĻص�����
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	//����ͼ�еİ�ť������ı�����  ͬ����
	UPROPERTY(meta = (BindWidget))	
	class UButton* HostButton;	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UFUNCTION()
		void HostButtonClicked();
	UFUNCTION()
		void JoinButtonClicked();
	UFUNCTION()
		void ExitButtonClicked();

	void MenuTearDown();

	//���ڴ������е����߻Ự���ܵ���ϵͳ
	class UMultiplayerSessionSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("")};
};
	
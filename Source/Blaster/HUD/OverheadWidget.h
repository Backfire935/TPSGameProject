// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
		UPROPERTY(meta = (BindWidget))//��ͼ��˭�������ü̳������C++�࣬˭�Ŀؼ���ͼ��ͬ���ؼ��ͺ������ͬ���ؼ�����
		class UTextBlock* DisplayText; //�ı���

		void SetDisplayText(FString TextToDisplay); //����Ҫչ�ֵ���������
		
		UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn *InPawn);	//������������ɫ
protected:
	virtual void OnLevelRemovedFromWorld(ULevel * InLevel, UWorld *InWorld) override;	//�Ƴ��ؼ�

};

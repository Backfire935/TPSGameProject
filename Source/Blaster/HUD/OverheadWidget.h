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
		UPROPERTY(meta = (BindWidget))//蓝图中谁的类设置继承了这个C++类，谁的控件蓝图的同名控件就和这里的同名控件绑定了
		class UTextBlock* DisplayText; //文本块

		void SetDisplayText(FString TextToDisplay); //设置要展现的文字内容
		
		UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn *InPawn);	//设置玩家网络角色
protected:
	virtual void OnLevelRemovedFromWorld(ULevel * InLevel, UWorld *InWorld) override;	//移除控件

};

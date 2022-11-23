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
	virtual void NativeDestruct() override;	//移除控件
	//UE5.1 删除OnLevelRemoveFromWorld函数，使用NativeDestruct函数替换，5.1版本以下换回OnLevelRemoveFromWorld，感谢epic讨论区老哥救我一命
	// https://forums.unrealengine.com/t/where-is-uuserwidget-onlevelremovedfromworld-in-5-1/692215/7
};

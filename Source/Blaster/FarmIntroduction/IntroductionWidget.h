// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiffResults.h"
#include"Blueprint/UserWidget.h"
#include"Components/RichTextBlock.h"
#include"Components/Overlay.h"
#include"Components/TextBlock.h"
#include"Components/BackgroundBlur.h"
#include "MediaPlayer.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include"IntroductionWidget.generated.h"

//如果需要打包制作插件的话，要把Blaster.Build.cs中的私有模块中的MediaAssets添加到插件的build.cs文件中的私有模块,在此提示
UENUM(BlueprintType)
enum class EIntroductionType : uint8
{
	Message,
	Modal
};

UCLASS()
class BLASTER_API UIntroductionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeConstruct() override;
	//文字引导的富文本框
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		URichTextBlock* MessageIntroductionText;
	//文字引导的覆层
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UOverlay* IntroductionOverlay;
	//文字引导的显示动画
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* ShowMessageAnim;
	//文字引导的隐藏动画
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* HideMessageAnim;


	//视频引导的画布
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UCanvasPanel* IntroductionModalCanvas;
	//视频引导的背景模糊
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UBackgroundBlur* Blur;
	//视频引导的富文本框
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		URichTextBlock* ModalIntroductionText;
	//视频引导的标题文本
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UTextBlock* IntroductionTitleText;
	//视频引导的图片，用来装视频
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UImage* ModalIntroductionMedia;
	//视频引导的进度条
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
	UProgressBar* ConfirmationProgressBar;
	//视频引导的按钮
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UButton* ConfirmButton;
	//视频引导显示的动画
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* ShowModalAnim;
	//视频引导隐藏的动画
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* HideModalAnim;

	//当前使用的引导类型是文字引导还是视频引导
	UPROPERTY(BlueprintReadWrite)
		EIntroductionType CurrentIntroductionType;

	//设置一个材质实例，获取视频添加到图片上从而播放视频 666
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UMaterialInstance* IntroductionMaterialInstance;

	//设置一个媒体播放器的变量
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UMediaPlayer* IntroductionMediaPlayer;
public:


	//显示文字引导界面
	UFUNCTION(BlueprintCallable)
		void ShowMessageIntroduction(FText InContent);
	//隐藏文字引导界面
	UFUNCTION(BlueprintCallable)
		void HideMessageIntroduction();

	//更新文字引导界面
	UFUNCTION(BlueprintCallable)
		void UpdateMessageIntroduction(FText NewContent);

	//显示视频引导界面
	UFUNCTION(BlueprintCallable)
		void ShowModalIntroduction(FName InTitle, FText InContent, UObject* InMedia);

	//更新视频引导界面
	void UpdateModalIntroduction(FText NewText);

	//按下按钮要保持的时间
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		float IntroductionHoldingTime = 2.f;
private:
	//按压按钮
	UFUNCTION()
		void HoldingStartButton();
	//松开按钮
	UFUNCTION()
		void ReleaseStartButton();

	//进度条变化率
	float HoldingPressBarChangeRate;
	//进度条插值
	float HoldingLerpAlpha = 2.f;


	//是否显示文字引导
		bool bIntroductionWordIsVisible = false;
	//是否显示视频引导
		bool bIsIntroductionVideoIsVisible = false;
	//是否开启tick
		bool bTick = false;


	//定时器
		FTimerHandle CollapsedTimer;

};


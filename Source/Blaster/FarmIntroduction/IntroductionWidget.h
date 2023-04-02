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

//�����Ҫ�����������Ļ���Ҫ��Blaster.Build.cs�е�˽��ģ���е�MediaAssets��ӵ������build.cs�ļ��е�˽��ģ��,�ڴ���ʾ
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
	//���������ĸ��ı���
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		URichTextBlock* MessageIntroductionText;
	//���������ĸ���
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UOverlay* IntroductionOverlay;
	//������������ʾ����
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* ShowMessageAnim;
	//�������������ض���
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* HideMessageAnim;


	//��Ƶ�����Ļ���
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UCanvasPanel* IntroductionModalCanvas;
	//��Ƶ�����ı���ģ��
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UBackgroundBlur* Blur;
	//��Ƶ�����ĸ��ı���
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		URichTextBlock* ModalIntroductionText;
	//��Ƶ�����ı����ı�
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UTextBlock* IntroductionTitleText;
	//��Ƶ������ͼƬ������װ��Ƶ
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UImage* ModalIntroductionMedia;
	//��Ƶ�����Ľ�����
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
	UProgressBar* ConfirmationProgressBar;
	//��Ƶ�����İ�ť
	UPROPERTY(BlueprintReadWrite, Category = "Default", meta = (BindWidget))
		UButton* ConfirmButton;
	//��Ƶ������ʾ�Ķ���
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* ShowModalAnim;
	//��Ƶ�������صĶ���
	UPROPERTY(BlueprintReadWrite, Category = "Animations", Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* HideModalAnim;

	//��ǰʹ�õ�������������������������Ƶ����
	UPROPERTY(BlueprintReadWrite)
		EIntroductionType CurrentIntroductionType;

	//����һ������ʵ������ȡ��Ƶ��ӵ�ͼƬ�ϴӶ�������Ƶ 666
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UMaterialInstance* IntroductionMaterialInstance;

	//����һ��ý�岥�����ı���
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		UMediaPlayer* IntroductionMediaPlayer;
public:


	//��ʾ������������
	UFUNCTION(BlueprintCallable)
		void ShowMessageIntroduction(FText InContent);
	//����������������
	UFUNCTION(BlueprintCallable)
		void HideMessageIntroduction();

	//����������������
	UFUNCTION(BlueprintCallable)
		void UpdateMessageIntroduction(FText NewContent);

	//��ʾ��Ƶ��������
	UFUNCTION(BlueprintCallable)
		void ShowModalIntroduction(FName InTitle, FText InContent, UObject* InMedia);

	//������Ƶ��������
	void UpdateModalIntroduction(FText NewText);

	//���°�ťҪ���ֵ�ʱ��
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		float IntroductionHoldingTime = 2.f;
private:
	//��ѹ��ť
	UFUNCTION()
		void HoldingStartButton();
	//�ɿ���ť
	UFUNCTION()
		void ReleaseStartButton();

	//�������仯��
	float HoldingPressBarChangeRate;
	//��������ֵ
	float HoldingLerpAlpha = 2.f;


	//�Ƿ���ʾ��������
		bool bIntroductionWordIsVisible = false;
	//�Ƿ���ʾ��Ƶ����
		bool bIsIntroductionVideoIsVisible = false;
	//�Ƿ���tick
		bool bTick = false;


	//��ʱ��
		FTimerHandle CollapsedTimer;

};


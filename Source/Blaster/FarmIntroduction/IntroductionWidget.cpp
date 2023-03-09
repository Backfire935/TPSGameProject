#include"IntroductionWidget.h"

#include "FileMediaSource.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/SphereComponent.h"


void UIntroductionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bTick) return;

	//1||0 * 单帧增加量*帧时间
	HoldingLerpAlpha += (HoldingLerpAlpha > 0 || HoldingPressBarChangeRate > 0) * HoldingPressBarChangeRate * InDeltaTime;
	//设置进度条
	ConfirmationProgressBar->SetPercent(HoldingLerpAlpha);

	if(HoldingLerpAlpha >= 1)
	{
		//自会判断是哪种类型
		HideMessageIntroduction();
		bTick = false;
	}
}

void UIntroductionWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	//设置UI为塌陷
	SetVisibility(ESlateVisibility::Collapsed);
	//游戏开始时看不到引导UI
	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);

	Blur->SetVisibility(ESlateVisibility::Collapsed);
	IntroductionModalCanvas->SetVisibility(ESlateVisibility::Collapsed);

	//绑定两个按钮
	ConfirmButton->OnPressed.AddDynamic(this, &UIntroductionWidget::HoldingStartButton);
	ConfirmButton->OnReleased.AddDynamic(this, &UIntroductionWidget::ReleaseStartButton);

}

void UIntroductionWidget::ShowMessageIntroduction(FText InContent)
{
	//可见性设为 
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	//不透明度设置为0
	MessageIntroductionText->SetRenderOpacity(0);
	//设置自动换行
	MessageIntroductionText->SetAutoWrapText(false);

	MessageIntroductionText->SetText(InContent);

	//计时器的另一种写法
	GetWorld()->GetTimerManager().SetTimer(
		*(new FTimerHandle()), 
		[this]()
		{
			if (!IsValidLowLevel()) return;

			MessageIntroductionText->SetRenderOpacity(1);
			MessageIntroductionText->SetAutoWrapText(true);
		},
		0.1,
		false
	);
	//end

	//要播放引导的话就播放动画
	if(!bIntroductionWordIsVisible)
	{
		PlayAnimation(ShowMessageAnim);
	}
	//播放过了所以改变布尔值
	bIntroductionWordIsVisible = true;

	GetWorld()->GetTimerManager().ClearTimer(CollapsedTimer);

}

void UIntroductionWidget::HideMessageIntroduction()
{
	if(CurrentIntroductionType == EIntroductionType::Modal && !bIsIntroductionVideoIsVisible || CurrentIntroductionType == EIntroductionType::Message && !bIntroductionWordIsVisible)
	{
		return;
	}

	UWidgetAnimation* AnimToPlay = CurrentIntroductionType == EIntroductionType::Message ? HideMessageAnim : HideModalAnim;

	if(IsAnimationPlaying(AnimToPlay))
	{
		return;
	}

	//播放隐藏动画
	PlayAnimation(AnimToPlay);

	float AnimLength = AnimToPlay->GetEndTime();

	//布尔变量当整数用，两个都为真时成立
	if(bIsIntroductionVideoIsVisible + bIntroductionWordIsVisible == 2)
	{
		//从当前种类动画转到另一种动画
		UWidgetAnimation* OtherAnimToPlay = CurrentIntroductionType == EIntroductionType::Message ? HideModalAnim : HideMessageAnim;

		PlayAnimation(OtherAnimToPlay);
		//拿到最长的动画播放时长
		if(OtherAnimToPlay->GetEndTime() > AnimLength)
		{
			AnimLength = OtherAnimToPlay->GetEndTime();
		}
	}

	GetWorld()->GetTimerManager().SetTimer(CollapsedTimer, [this]()
		{
			//设置UI为塌陷的以节省性能
			SetVisibility(ESlateVisibility::Collapsed);
		},
		AnimLength + 0.5,
		false
		);
	//end

	if(bIsIntroductionVideoIsVisible)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		PlayerController->SetPause(false);
		PlayerController->SetShowMouseCursor(false);
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);

	}
	
	bIsIntroductionVideoIsVisible = false;
	bIntroductionWordIsVisible = false;

}

void UIntroductionWidget::UpdateMessageIntroduction(FText NewContent)
{
	//if(MessageIntroductionText->GetText().ToString().Len() == NewContent.ToString().Len()){}
	//如果有新内容就刷新字符
	if(MessageIntroductionText->GetText().ToString().Compare(NewContent.ToString(),ESearchCase::IgnoreCase))
	{
		//拼接字符串
		NewContent = FText::FromString(NewContent.ToString() + TEXT(" "));
	}
	MessageIntroductionText->SetText(NewContent);
}

void UIntroductionWidget::ShowModalIntroduction(FName InTitle, FText InContent, UObject* InMedia)
{
	if(!ModalIntroductionMedia || !IntroductionMediaPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("%s 的视频文件或视频播放器不存在"),*InTitle.ToString());
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	//设置暂停
	Controller->SetPause(true);
	//显示鼠标
	Controller->SetShowMouseCursor(true);
	//设置游戏输入模式为游戏和UI
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(Controller);
	//设置标题
	IntroductionTitleText->SetText(FText::FromName(InTitle));
	//设置内容
	ModalIntroductionText->SetText(InContent);

	//媒体播放器是否存在
	if(InMedia)
	{
		//媒体播放器是否为有效的媒体源
		if (InMedia->GetClass()->IsChildOf<UFileMediaSource>())
		{
			//媒体播放器打开媒体资源
			IntroductionMediaPlayer->OpenSource(Cast<UFileMediaSource>(InMedia));
			//图片设置材质实例
			ModalIntroductionMedia->SetBrushFromMaterial(IntroductionMaterialInstance);
		}
		else
		{
			//设置画笔资源对象
			ModalIntroductionMedia->SetBrushResourceObject(InMedia);
		}
	}//媒体不存在
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s 的播放器不存在"),*InTitle.ToString());
	}

	//设置当前引导类型
	CurrentIntroductionType = EIntroductionType::Modal;
	if(!bIsIntroductionVideoIsVisible)
	{
		PlayAnimation(ShowModalAnim);
	}
	HoldingLerpAlpha = 0;
	//进度条
	ConfirmationProgressBar->SetPercent(0);
	//播放设为真
	bIsIntroductionVideoIsVisible = true;
	GetWorld()->GetTimerManager().ClearTimer(CollapsedTimer);
}

void UIntroductionWidget::UpdateModalIntroduction(FText NewText)
{
	if (ModalIntroductionText->GetText().ToString().Compare(NewText.ToString(), ESearchCase::IgnoreCase))
	{
		//拼接字符串
		NewText = FText::FromString(NewText.ToString() + TEXT(" "));
	}
	ModalIntroductionText->SetText(NewText);
}

void UIntroductionWidget::HoldingStartButton()
{
	if (!bIsIntroductionVideoIsVisible) return;
	//设置进度条增长率
	HoldingPressBarChangeRate = 1 / IntroductionHoldingTime;
	//开启tick
	bTick = true;

}

void UIntroductionWidget::ReleaseStartButton()
{
	HoldingPressBarChangeRate = -2;
}

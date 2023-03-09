#include"IntroductionWidget.h"

#include "FileMediaSource.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/SphereComponent.h"


void UIntroductionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bTick) return;

	//1||0 * ��֡������*֡ʱ��
	HoldingLerpAlpha += (HoldingLerpAlpha > 0 || HoldingPressBarChangeRate > 0) * HoldingPressBarChangeRate * InDeltaTime;
	//���ý�����
	ConfirmationProgressBar->SetPercent(HoldingLerpAlpha);

	if(HoldingLerpAlpha >= 1)
	{
		//�Ի��ж�����������
		HideMessageIntroduction();
		bTick = false;
	}
}

void UIntroductionWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	//����UIΪ����
	SetVisibility(ESlateVisibility::Collapsed);
	//��Ϸ��ʼʱ����������UI
	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);

	Blur->SetVisibility(ESlateVisibility::Collapsed);
	IntroductionModalCanvas->SetVisibility(ESlateVisibility::Collapsed);

	//��������ť
	ConfirmButton->OnPressed.AddDynamic(this, &UIntroductionWidget::HoldingStartButton);
	ConfirmButton->OnReleased.AddDynamic(this, &UIntroductionWidget::ReleaseStartButton);

}

void UIntroductionWidget::ShowMessageIntroduction(FText InContent)
{
	//�ɼ�����Ϊ 
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	//��͸��������Ϊ0
	MessageIntroductionText->SetRenderOpacity(0);
	//�����Զ�����
	MessageIntroductionText->SetAutoWrapText(false);

	MessageIntroductionText->SetText(InContent);

	//��ʱ������һ��д��
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

	//Ҫ���������Ļ��Ͳ��Ŷ���
	if(!bIntroductionWordIsVisible)
	{
		PlayAnimation(ShowMessageAnim);
	}
	//���Ź������Ըı䲼��ֵ
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

	//�������ض���
	PlayAnimation(AnimToPlay);

	float AnimLength = AnimToPlay->GetEndTime();

	//���������������ã�������Ϊ��ʱ����
	if(bIsIntroductionVideoIsVisible + bIntroductionWordIsVisible == 2)
	{
		//�ӵ�ǰ���ද��ת����һ�ֶ���
		UWidgetAnimation* OtherAnimToPlay = CurrentIntroductionType == EIntroductionType::Message ? HideModalAnim : HideMessageAnim;

		PlayAnimation(OtherAnimToPlay);
		//�õ���Ķ�������ʱ��
		if(OtherAnimToPlay->GetEndTime() > AnimLength)
		{
			AnimLength = OtherAnimToPlay->GetEndTime();
		}
	}

	GetWorld()->GetTimerManager().SetTimer(CollapsedTimer, [this]()
		{
			//����UIΪ���ݵ��Խ�ʡ����
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
	//����������ݾ�ˢ���ַ�
	if(MessageIntroductionText->GetText().ToString().Compare(NewContent.ToString(),ESearchCase::IgnoreCase))
	{
		//ƴ���ַ���
		NewContent = FText::FromString(NewContent.ToString() + TEXT(" "));
	}
	MessageIntroductionText->SetText(NewContent);
}

void UIntroductionWidget::ShowModalIntroduction(FName InTitle, FText InContent, UObject* InMedia)
{
	if(!ModalIntroductionMedia || !IntroductionMediaPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("%s ����Ƶ�ļ�����Ƶ������������"),*InTitle.ToString());
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	//������ͣ
	Controller->SetPause(true);
	//��ʾ���
	Controller->SetShowMouseCursor(true);
	//������Ϸ����ģʽΪ��Ϸ��UI
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(Controller);
	//���ñ���
	IntroductionTitleText->SetText(FText::FromName(InTitle));
	//��������
	ModalIntroductionText->SetText(InContent);

	//ý�岥�����Ƿ����
	if(InMedia)
	{
		//ý�岥�����Ƿ�Ϊ��Ч��ý��Դ
		if (InMedia->GetClass()->IsChildOf<UFileMediaSource>())
		{
			//ý�岥������ý����Դ
			IntroductionMediaPlayer->OpenSource(Cast<UFileMediaSource>(InMedia));
			//ͼƬ���ò���ʵ��
			ModalIntroductionMedia->SetBrushFromMaterial(IntroductionMaterialInstance);
		}
		else
		{
			//���û�����Դ����
			ModalIntroductionMedia->SetBrushResourceObject(InMedia);
		}
	}//ý�岻����
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s �Ĳ�����������"),*InTitle.ToString());
	}

	//���õ�ǰ��������
	CurrentIntroductionType = EIntroductionType::Modal;
	if(!bIsIntroductionVideoIsVisible)
	{
		PlayAnimation(ShowModalAnim);
	}
	HoldingLerpAlpha = 0;
	//������
	ConfirmationProgressBar->SetPercent(0);
	//������Ϊ��
	bIsIntroductionVideoIsVisible = true;
	GetWorld()->GetTimerManager().ClearTimer(CollapsedTimer);
}

void UIntroductionWidget::UpdateModalIntroduction(FText NewText)
{
	if (ModalIntroductionText->GetText().ToString().Compare(NewText.ToString(), ESearchCase::IgnoreCase))
	{
		//ƴ���ַ���
		NewText = FText::FromString(NewText.ToString() + TEXT(" "));
	}
	ModalIntroductionText->SetText(NewText);
}

void UIntroductionWidget::HoldingStartButton()
{
	if (!bIsIntroductionVideoIsVisible) return;
	//���ý�����������
	HoldingPressBarChangeRate = 1 / IntroductionHoldingTime;
	//����tick
	bTick = true;

}

void UIntroductionWidget::ReleaseStartButton()
{
	HoldingPressBarChangeRate = -2;
}

#include"IntroductionWidget.h"

 void UIntroductionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//设置UI为塌陷
	SetVisibility(ESlateVisibility::Collapsed);
	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);
	//游戏开始时看不到引导UI


}

void UIntroductionWidget::ShowMessageIntroduction(FText InContent)
{
	//可见性设为 
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	//不透明度设置为0
	IntroductionOverlay->SetRenderOpacity(0);
	//设置自动换行
	IntroductionOverlay->SetAutoWrapText(false);

	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UIntroductionWidget::HideMessageIntroduction()
{
}

void UIntroductionWidget::UpdateMessageIntroduction(FText NewContent)
{
}

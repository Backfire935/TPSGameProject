#include"IntroductionWidget.h"

 void UIntroductionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//����UIΪ����
	SetVisibility(ESlateVisibility::Collapsed);
	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);
	//��Ϸ��ʼʱ����������UI


}

void UIntroductionWidget::ShowMessageIntroduction(FText InContent)
{
	//�ɼ�����Ϊ 
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	//��͸��������Ϊ0
	IntroductionOverlay->SetRenderOpacity(0);
	//�����Զ�����
	IntroductionOverlay->SetAutoWrapText(false);

	IntroductionOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UIntroductionWidget::HideMessageIntroduction()
{
}

void UIntroductionWidget::UpdateMessageIntroduction(FText NewContent)
{
}

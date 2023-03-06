#pragma once
#include"CoreMinimal.h"
#include"Blueprint/UserWidget.h"
#include"Components/RichTextBlock.h"
#include"Components/Overlay.h"

#include"IntroductionWidget.h"

UCLASS(Abstract)
class FARMINTRODUCTION_API UIntroductionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, Catrgory = "Default", meta = (BindWidget))
		URichTextBlock* MessagIntroductionText;

	UPROPERTY(BlueprintReadWrite, Catrgory = "Default", meta = (BindWidget))
		UOverlay* IntroductionOverlay;

	UPROPERTY(BlueprintReadWrite, Transient, Catrgory = "Animations", meta = (BindWidget))
		UWidgetAnimation* ShowMessageAnim;

	UPROPERTY(BlueprintReadWrite, Transient, Catrgory = "Animations", meta = (BindWidget))
		UWidgetAnimation* HideMessageAnim;

public:
	//��ʾ����
	UFUNCTION(BlueprintCallable)
		void ShowMessageIntroduction(FText InContent);
	
	//��������
	UFUNCTION(BlueprintCallable)
		void HideMessageIntroduction();

	//��������
	UFUNCTION(BlueprintCallable)
		void UpdateMessageIntroduction(FText NewContent);

private:
	//�Ƿ�������������
	UPROPERTY(BlueprintReadWrite)
	bool bIntroductionIsVisible = false;

	//����������ڵĶ�ʱ��
	UPROPERTY(BlueprintReadWrite)
		FTimerHandle CollapsedTimer;
};



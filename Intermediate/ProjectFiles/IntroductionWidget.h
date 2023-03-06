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
	//显示引导
	UFUNCTION(BlueprintCallable)
		void ShowMessageIntroduction(FText InContent);
	
	//隐藏引导
	UFUNCTION(BlueprintCallable)
		void HideMessageIntroduction();

	//更新引导
	UFUNCTION(BlueprintCallable)
		void UpdateMessageIntroduction(FText NewContent);

private:
	//是否隐藏引导界面
	UPROPERTY(BlueprintReadWrite)
	bool bIntroductionIsVisible = false;

	//引导界面存在的定时器
	UPROPERTY(BlueprintReadWrite)
		FTimerHandle CollapsedTimer;
};



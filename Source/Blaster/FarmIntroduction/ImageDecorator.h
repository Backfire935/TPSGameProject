// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "Components/RichTextBlockImageDecorator.h"
#include "ImageDecorator.generated.h"

//编译slate需要在build.cs里面的private模块加上"Slate", "SlateCore"
/**
 * 
 */
UCLASS()
class BLASTER_API UImageDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:
	UImageDecorator(const FObjectInitializer& ObjectInitializer);

	//创建一个装饰器
	virtual  TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	//获取图片笔刷
	virtual const FSlateBrush* FindImageBrush(FName TagOrId, bool bWarnIfMissing);

	float GetRelativeImageScale() { return RelativeImageScale; }

protected:
	UPROPERTY(EditAnywhere, Category = Appearance, meta = (RowType = "RichImageRow"))
		float RelativeImageScale = 1.5f;

	UPROPERTY(EditAnywhere, Category = Appearance, meta = (RowType = "RichImageRow"))
		class UDataTable* ImageSet;

	FRichImageRow* FindImageRow(FName TagOrId, bool bWarnIfMissing);
};

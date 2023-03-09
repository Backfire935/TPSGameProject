// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageDecorator.h"
#include"Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Fonts/FontMeasure.h"
#include "Misc/DefaultValueHelper.h"

class SRichInlineImage : public  SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRichInlineImage)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const FSlateBrush* Brush, const FTextBlockStyle& TextStyle, TOptional<int32>Width, TOptional<int32>Height, float FontScale) 
	{
		if(ensure(Brush))
		{
			const TSharedRef<FSlateFontMeasure>FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			float TextHeight = (float)FontMeasure->GetMaxCharacterHeight(TextStyle.Font, 1);
			float IconHeight = TextHeight * FontScale;
			float IconWidth = (IconHeight / Brush->ImageSize.Y) * Brush->ImageSize.X;

			if(Width.IsSet())
			{
				IconWidth = Width.GetValue();
			}
			if (Height.IsSet())
			{
				IconHeight = Height.GetValue();
			}

			ChildSlot
				[
					SNew(SBox)
					.HeightOverride(IconHeight)
				.WidthOverride(IconWidth)
				.RenderTransform(FTransform2D(FVector2D(0, (IconHeight - TextHeight) / 2)))
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(Brush)
				]
				]
				];
		}
	}
};

class FRichInLineImage : public FRichTextDecorator
{
public:
	FRichInLineImage(URichTextBlock* InOwner, UImageDecorator* InDecorator)
		: FRichTextDecorator(InOwner)
		, Decorator(InDecorator)
	{
		
	}

	//指定装饰器处理哪些类型的标签
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override
	{
		//是否包含名字为img或者关键字为id的项
		if (RunParseResult.Name == TEXT("img") && RunParseResult.MetaData.Contains(TEXT("id")))
		{
			const FTextRange& IdRange = RunParseResult.MetaData[TEXT("id")];
			const FString TagId = Text.Mid(IdRange.BeginIndex, IdRange.EndIndex - IdRange.BeginIndex);

			const bool bWarnIfMissing = false;
			return Decorator->FindImageBrush(*TagId, bWarnIfMissing) != nullptr;
		}
		return false;
	}



protected:
	//创建装饰器界面,比如image
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo, const FTextBlockStyle& DefaultTextStyle) const override
	{
		const bool bWarnIfMissing = true;

		const FSlateBrush* Brush = Decorator->FindImageBrush(*RunInfo.MetaData[TEXT("id")], bWarnIfMissing);

		//拿到图片笔刷的宽和高
		//宽度
		TOptional<int32> Width;
		if (const FString* WidthString = RunInfo.MetaData.Find(TEXT("width")))
		{
			int32 WidthTemp;

			Width = FDefaultValueHelper::ParseInt(*WidthString, WidthTemp) ? WidthTemp : TOptional<int32>();
		}
		//高度
		TOptional<int32> Height;
		if (const FString* HeightString = RunInfo.MetaData.Find(TEXT("height")))
		{
			int32 HeightTemp;

			Height = FDefaultValueHelper::ParseInt(*HeightString, HeightTemp) ? HeightTemp : TOptional<int32>();
		}

		return SNew(SRichInlineImage, Brush, DefaultTextStyle, Width, Height, Decorator->GetRelativeImageScale());
	}

	UImageDecorator* Decorator;
};

UImageDecorator::UImageDecorator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

TSharedPtr<ITextDecorator> UImageDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FRichInLineImage(InOwner, this));
}


const FSlateBrush* UImageDecorator::FindImageBrush(FName TagOrId, bool bWarnIfMissing)
{
	const FRichImageRow * ImageRow = FindImageRow(TagOrId, bWarnIfMissing);
	if(ImageRow)
	{
		return &ImageRow->Brush;
	}
	//不存在返回空
	return nullptr;
}

FRichImageRow* UImageDecorator::FindImageRow(FName TagOrId, bool bWarnIfMissing)
{
	if(ImageSet)
	{
		FString ContextString;
		return ImageSet->FindRow<FRichImageRow>(TagOrId, ContextString, bWarnIfMissing);
	}
	return  nullptr;
}

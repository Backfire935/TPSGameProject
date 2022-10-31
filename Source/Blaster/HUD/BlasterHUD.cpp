// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include"GameFramework/PlayerController.h"
#include"CharacterOverlay.h"
#include"Announcement.h"
void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X /2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairsSpread;//准心偏移的浮点量
		FVector2D Spread;//准心偏移坐标
		if (HUDPackage.CrosshairsCenter)//开始画上下左右和中心的准心纹理
		{
			Spread = FVector2D(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			Spread = FVector2D(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsRight)
		{
			Spread = FVector2D(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsTop)
		{
			Spread = FVector2D(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsBottom)
		{
			Spread = FVector2D(0.f, SpreadScaled); 
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();//获取拥有的玩家控制器
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);//创建控件到角色上
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()//此函数由玩家控制器在经过开局倒计时后进入游戏状态后调用
{
	APlayerController * PlayerController = GetOwningPlayerController();//获取拥有的玩家控制器
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);//创建控件到角色上
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();//要设置的准心的纹理宽度等于从武器类那拿到的纹理的尺寸的宽度
	const float TextureHeight = Texture->GetSizeY();//同上
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f +Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f +Spread.Y
	);

	DrawTexture(
		Texture,//纹理
		TextureDrawPoint.X,//屏幕尺寸X
		TextureDrawPoint.Y,//屏幕尺寸Y
		TextureWidth,//纹理宽度
		TextureHeight,//纹理高度
		0.f,//纹理U
		0.f,//纹理V
		1.f,//纹理U宽度
		1.f,//纹理V高度
		CrosshairColor//画出来的纹理颜色
	);
}

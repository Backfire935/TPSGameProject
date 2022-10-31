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
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairsSpread;//׼��ƫ�Ƶĸ�����
		FVector2D Spread;//׼��ƫ������
		if (HUDPackage.CrosshairsCenter)//��ʼ���������Һ����ĵ�׼������
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
	APlayerController* PlayerController = GetOwningPlayerController();//��ȡӵ�е���ҿ�����
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);//�����ؼ�����ɫ��
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()//�˺�������ҿ������ھ������ֵ���ʱ�������Ϸ״̬�����
{
	APlayerController * PlayerController = GetOwningPlayerController();//��ȡӵ�е���ҿ�����
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);//�����ؼ�����ɫ��
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();//Ҫ���õ�׼�ĵ������ȵ��ڴ����������õ�������ĳߴ�Ŀ��
	const float TextureHeight = Texture->GetSizeY();//ͬ��
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f +Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f +Spread.Y
	);

	DrawTexture(
		Texture,//����
		TextureDrawPoint.X,//��Ļ�ߴ�X
		TextureDrawPoint.Y,//��Ļ�ߴ�Y
		TextureWidth,//������
		TextureHeight,//����߶�
		0.f,//����U
		0.f,//����V
		1.f,//����U���
		1.f,//����V�߶�
		CrosshairColor//��������������ɫ
	);
}

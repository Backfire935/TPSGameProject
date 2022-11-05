

#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)//֡���
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.time = GetWorld()->GetTimeSeconds();//֡�������ʱ��
		for(auto & BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation  BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();//Value�������
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();//��ȡ��������С
			//�����Ϣ
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);// HitBoxInfo�Ľṹ��TMap<FName, FBoxInformation>������ʹ��BoxPair.Key
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderPackage,
	const FFramePackage& YoungerPackage, float HitTime)
{
	const float Distance =  (YoungerPackage.time - OlderPackage.time);//������֮���ʱ��
	const float InterpFraction = FMath::Clamp(HitTime - OlderPackage.time / Distance, 0 ,1);// ���е���һ����֮���ʱ��/������֮���ʱ��

	FFramePackage InterpFramePackage;
	InterpFramePackage.time = HitTime;

	for(auto& YoungerPair : YoungerPackage.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderPackage.HitBoxInfo[BoxInfoName];//��ȡvalue
		const FBoxInformation& YoungerBox = YoungerPackage.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;//ģ���������Ŀͻ�������ʱ����Ϣλ��
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location,YoungerBox.Location, 1.f, InterpFraction);//��ֵ�������location��Ϣ
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);//��ֵ�������rotation��Ϣ
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;//�����С

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);//���
	}

	return InterpFramePackage;

}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter ==nullptr ) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);//�����ɫ��ǰ��λ����Ϣ�����ذ�
	MoveBoxes(HitCharacter, Package);//�ƶ�hit���ӵ��ͻ��˷����İ���λ����Ϣ
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);//�ص���ɫģ�͵���ײ,���������hitbox��ײ���߼�������

	//�ж�������hitbox����ײ����߼�
	//�ȴ���ͷ��hitbox����ײ
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//��ײ����
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//��ײ����

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25;
	UWorld *World = GetWorld();
	if(World)
	{//������ײ��⣬��Ϊֻ����ͷ������ײ������ֻҪ�������Ǿ�������ͷ��
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);
		if(ConfirmHitResult.bBlockingHit)//����ͷ������ǰ����
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);//����ɫ��λ���ƻ�ȥ
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//������˾ʹ�ģ����ײ����Ȼ��ϷҪ����
			return FServerSideRewindResult{ true , true };
		}
		else//û������ͷ���Ļ�����ȥ���ʣ�µ����岿λ
		{
			for(auto & HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					//�Ѿ�����ͷ���ˣ���������к����Ѿ�return�������е���Ϳ϶���������ͷ��������ֱ�ӿ������е���ײ���ɣ����ҷ��صĽ���Ѿ���ȷ����
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
				World->LineTraceSingleByChannel(
					ConfirmHitResult,
					TraceStart,
					TraceEnd,
					ECollisionChannel::ECC_Visibility
				);
				if(ConfirmHitResult.bBlockingHit)
				{
					ResetHitBoxes(HitCharacter, CurrentFrame);//����ɫ��λ���ƻ�ȥ
					EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//������˾ʹ�ģ����ײ����Ȼ��ϷҪ����
					return FServerSideRewindResult{ true , false };
				}
			}
		}
	}
	//�������ȷ��û������Ŀ��,�ص��������ԣ���������false
	ResetHitBoxes(HitCharacter, CurrentFrame);//����ɫ��λ���ƻ�ȥ
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//������˾ʹ�ģ����ײ����Ȼ��ϷҪ����
	return FServerSideRewindResult{ false , false };
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)//������ɫȫ�����ײ����
	{
		if(HitBoxPair.Value != nullptr )
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key,BoxInfo);//���,ÿһ�����ӵ����ֺͺ��ӵ���Ϣ
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr)return;
	for(auto &HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)//value�Ǿ������ײ����
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//���������λ����Ϣ��
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//�����������ת��Ϣ��
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//��������Ĵ�С
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)//����ɫ��λ���ƻ�ȥ
{
	if (HitCharacter == nullptr)return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)//value�Ǿ������ײ����
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//���������λ����Ϣ��
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//�����������ת��Ϣ��
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//��������Ĵ�С
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);//ֱ�ӹص�head hit box��ײ
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}


void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)//��ʾ������
{
	for(auto & BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			5.0f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)//������Ϣ������˻��ݵĺ���
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if(bReturn) return FServerSideRewindResult();

	bool bShouldInterpolate = true;//�Ƿ�ʹ�ò�֡����

	//�������ڼ�����е�֡��
	FFramePackage FrameToCheck;

	//�����е���ҵ�֡��ʷ
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	//֡��ʷ�����һ֡������ʱ��
	const float OldestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail()->GetValue().time;
	//֡��ʷ������һ֡������ʱ��
	const float NewestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead()->GetValue().time;

	if(OldestHistoryTime > HitTime)//�������ʱ�䷢��������ļ�¼���磬����������Ч����Ϊ�Ѿ��������ݵķ�Χ��
	{
		return FServerSideRewindResult();
	}

	if(OldestHistoryTime == HitTime)//������е�ʱ��ǡ�õ������һ֡����ʱ��
	{
		FrameToCheck = History.GetTail()->GetValue();//Ҫ����֡��Ϊ���һ��֡��
		bShouldInterpolate = false;
	}

	if(NewestHistoryTime <= HitTime)//������´洢��ʱ������е�ʱ�仹С,�����������ܷ������������ӳ٣�˵��׼�������õ�һ����ȥ�жϾ�����
	{
		FrameToCheck = History.GetHead()->GetValue();//ֱ�Ӽ���һ��֡��
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode * Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetTail();
	while(Older->GetValue().time >HitTime)//�����ǰolder�ڵ��ʱ�������ʱ�仹��
	{
		if (Older->GetNextNode() == nullptr) break;//����¸��ڵ��ǿսڵ�Ļ����ͽ���ѭ���ص�����
		Younger = Older;//��young�ڵ��Ƶ��˴�
		Older = Older->GetNextNode();//��older�ڵ���ǰ��
	}//���Ľ����Ҫolder�ڵ�ʱ���hittime�磬young�ڵ�ʱ���hittime��

	//����������������ܵ����Ǽ���
	if(Older->GetValue().time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		//������ò�֡����
		FrameToCheck = InterpBetweenFrames(Older->GetValue(),Younger->GetValue() , HitTime);
	}

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
	
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation,float HitTime, class AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(Character && HitCharacter &&  DamageCauser && Confirm.bHitComfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();


}

void ULagCompensationComponent::SaveFramePackage()
{
	//ֻ�з���˲���ִ��
	if (Character == nullptr || !Character->HasAuthority())return;

	if (FrameHistory.Num() <= 1)//��һ֡�������
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//���µ�һ֡
		FrameHistory.AddHead(ThisFrame);//��ӵ�������
	}
	else//�����һ֡���ϵ����
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;//�洢��ǰ��֡����Ϊ���µ�ʱ��-��ɵ�ʱ�䣬�������ʱ���Ƚ�
		while (HistoryLength > MaxRecordTime)//��ǰ��֡����>����¼ʱ��Ļ���������������¼ʱ��Ĳ���
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;
		}
		//��������Ҫ�ĺ󣬾Ϳ��Լ��������µ�����
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//��ȡ��ǰ֡
		FrameHistory.AddHead(ThisFrame);//��ǰ֡��ӵ�������

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}



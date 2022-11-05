

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

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)//帧打包
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.time = GetWorld()->GetTimeSeconds();//帧包中添加时间
		for(auto & BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation  BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();//Value就是组件
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();//获取组件盒体大小
			//打包信息
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);// HitBoxInfo的结构是TMap<FName, FBoxInformation>，所以使用BoxPair.Key
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderPackage,
	const FFramePackage& YoungerPackage, float HitTime)
{
	const float Distance =  (YoungerPackage.time - OlderPackage.time);//两个包之间的时间
	const float InterpFraction = FMath::Clamp(HitTime - OlderPackage.time / Distance, 0 ,1);// 命中到上一个包之间的时间/两个包之间的时间

	FFramePackage InterpFramePackage;
	InterpFramePackage.time = HitTime;

	for(auto& YoungerPair : YoungerPackage.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderPackage.HitBoxInfo[BoxInfoName];//获取value
		const FBoxInformation& YoungerBox = YoungerPackage.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;//模拟计算出来的客户端命中时的信息位置
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location,YoungerBox.Location, 1.f, InterpFraction);//插值算出来的location信息
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);//插值算出来的rotation信息
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;//盒体大小

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);//封包
	}

	return InterpFramePackage;

}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter ==nullptr ) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);//缓存角色当前的位置信息到本地包
	MoveBoxes(HitCharacter, Package);//移动hit盒子到客户端发来的包的位置信息
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);//关掉角色模型的碰撞,避免下面和hitbox碰撞的逻辑出问题

	//判断射线与hitbox的碰撞检测逻辑
	//先打开与头部hitbox的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//碰撞开关
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//碰撞种类

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25;
	UWorld *World = GetWorld();
	if(World)
	{//射线碰撞检测，因为只开了头部的碰撞，所以只要碰到了那就是碰到头了
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);
		if(ConfirmHitResult.bBlockingHit)//命中头部，提前结束
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
			return FServerSideRewindResult{ true , true };
		}
		else//没有命中头部的话，就去检测剩下的身体部位
		{
			for(auto & HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if(HitBoxPair.Value != nullptr)
				{
					//已经检测过头部了，如果能命中函数已经return，能运行到这就肯定不能命中头部，所以直接开启所有的碰撞即可，而且返回的结果已经被确定了
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
					ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
					EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
					return FServerSideRewindResult{ true , false };
				}
			}
		}
	}
	//如果检测后确定没有命中目标,关掉所有属性，返回两个false
	ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
	return FServerSideRewindResult{ false , false };
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)//遍历角色全身的碰撞盒子
	{
		if(HitBoxPair.Value != nullptr )
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key,BoxInfo);//封包,每一个盒子的名字和盒子的信息
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr)return;
	for(auto &HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)//value是具体的碰撞盒子
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//设置组件的位置信息到
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//设置组件的旋转信息到
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//设置组件的大小
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)//将角色的位置移回去
{
	if (HitCharacter == nullptr)return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)//value是具体的碰撞盒子
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//设置组件的位置信息到
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//设置组件的旋转信息到
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//设置组件的大小
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);//直接关掉head hit box碰撞
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


void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)//显示包内容
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
	const FVector_NetQuantize& HitLocation, float HitTime)//发送信息给服务端回溯的函数
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if(bReturn) return FServerSideRewindResult();

	bool bShouldInterpolate = true;//是否使用插帧计算

	//我们用于检查命中的帧包
	FFramePackage FrameToCheck;

	//被命中的玩家的帧历史
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	//帧历史中最后一帧发生的时间
	const float OldestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail()->GetValue().time;
	//帧历史中最新一帧发生的时间
	const float NewestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead()->GetValue().time;

	if(OldestHistoryTime > HitTime)//如果命中时间发生比最早的纪录还早，本次命中无效，因为已经超出回溯的范围了
	{
		return FServerSideRewindResult();
	}

	if(OldestHistoryTime == HitTime)//如果命中的时间恰好等于最后一帧包的时间
	{
		FrameToCheck = History.GetTail()->GetValue();//要检查的帧包为最后一个帧包
		bShouldInterpolate = false;
	}

	if(NewestHistoryTime <= HitTime)//如果最新存储的时间比命中的时间还小,听起来不可能发生但是由于延迟，说不准，反正用第一个包去判断就是了
	{
		FrameToCheck = History.GetHead()->GetValue();//直接检查第一个帧包
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode * Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetTail();
	while(Older->GetValue().time >HitTime)//如果当前older节点的时间比命中时间还晚
	{
		if (Older->GetNextNode() == nullptr) break;//如果下个节点是空节点的话，就结束循环回到函数
		Younger = Older;//让young节点移到此处
		Older = Older->GetNextNode();//让older节点往前移
	}//最后的结果是要older节点时间比hittime早，young节点时间比hittime晚

	//这种情况几乎不可能但还是加上
	if(Older->GetValue().time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		//这里调用插帧函数
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
	//只有服务端才能执行
	if (Character == nullptr || !Character->HasAuthority())return;

	if (FrameHistory.Num() <= 1)//第一帧的情况下
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//存下第一帧
		FrameHistory.AddHead(ThisFrame);//添加到链表中
	}
	else//添加了一帧以上的情况
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;//存储当前的帧长度为最新的时间-最旧的时间，后面和总时长比较
		while (HistoryLength > MaxRecordTime)//当前的帧长度>最大纪录时间的话，丢掉超出最大纪录时间的部分
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;
		}
		//丢掉不需要的后，就可以继续更新新的内容
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//获取当前帧
		FrameHistory.AddHead(ThisFrame);//当前帧添加到链表中

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}



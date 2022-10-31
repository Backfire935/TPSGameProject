// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	return EHandleBlockingHitResult::AdvanceNextSubstep;//ֱ�����������ײ
	//������һ��ģ����¡�ͨ���ڿ��Ժ�����ײ���λص����߼�ʱʹ�ã����統һ����ס��Ͷ��������屻�ݻ�ʱ���˶�Ӧ�ü���
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//�����Ӧ��ͣ������ֻ������ײ���Ӽ�⵽��ײ���Żᱬը
	
}

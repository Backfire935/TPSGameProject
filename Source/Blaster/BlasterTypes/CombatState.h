#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),//���е�

	ECS_Reloading UMETA(DisplayName = "Reloading"),//����

	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),//����

	ECS_Max UMETA(DisplayName = "DefaultMax")

};
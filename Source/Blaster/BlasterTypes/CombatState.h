#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),//¿ÕÏÐµÄ

	ECS_Reloading UMETA(DisplayName = "Reloading"),//»»µ¯

	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),//»»µ¯

	ECS_Max UMETA(DisplayName = "DefaultMax")

};
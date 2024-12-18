#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	RTIP_NoTurning UMETA(DisplayName = "No Turning"),
	RTIP_TurnLeft  UMETA(DisplayName = "Turn Left"),
	RTIP_TurnRight UMETA(DisplayName = "Turn Right"),

	RTIP_MAX       UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unoccupied  UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Throwing UMETA(DisplayName = "Throwing"),
	ECS_Swapping UMETA(DisplayName = "Swapping"),

	ECS_MAX       UMETA(DisplayName = "DefaultMAX")
};



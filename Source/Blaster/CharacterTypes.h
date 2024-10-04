#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	RTIP_NoTurning UMETA(DisplayName = "No Turning"),
	RTIP_TurnLeft  UMETA(DisplayName = "Turn Left"),
	RTIP_TurnRight UMETA(DisplayName = "Turn Right"),

	RTIP_MAX       UMETA(DisplayName = "DefaultMAX")
};




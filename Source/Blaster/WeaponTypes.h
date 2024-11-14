#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Attack Rifle"),
	EWT_Rocket UMETA(DisplayName = "Rocket"), //NEW1

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

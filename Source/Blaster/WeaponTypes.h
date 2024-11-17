#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Attack Rifle"),
	EWT_Rocket UMETA(DisplayName = "Rocket"), //NEW1
	EWT_Pistol UMETA(DisplayName = "Pistol"), //NEW2
	EWT_SMG UMETA(DisplayName = "SMG"), //NEW3

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

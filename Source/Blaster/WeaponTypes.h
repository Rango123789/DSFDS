#pragma once

#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	//Projectile weapon
	EWT_AssaultRifle UMETA(DisplayName = "Attack Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"), //NEW1

	//HitScan weapon
	EWT_Pistol UMETA(DisplayName = "Pistol"), //NEW2
	EWT_SMG UMETA(DisplayName = "SMG"), //NEW3
	EWT_Shotgun UMETA(DisplayName = "Shotgun"), //NEW4
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"), //NEW5
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"), //NEW6

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"), //Stephen add "Initial State"
	EWS_Equipped UMETA(DisplayName = "Equipped"),      //for active weapon
	EWS_EquippedSecond UMETA(DisplayName = "EquippedSecond"), //for weapon on backpack

	EWS_Droppped UMETA(DisplayName = "Dropped"),

	EWP_MAX UMETA(DisplayName = "DefaultMAX"), //for the sake of knowing now many sematic values of this enum
};

//this is OPTIONAL:
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_Projectile UMETA(DisplayName = "Projectile Firing"),
	EFT_HitScan UMETA(DisplayName = "HitScan Firing"), //NEW3
	EFT_Shotgun UMETA(DisplayName = "Shotgun Firing"), //NEW4

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};




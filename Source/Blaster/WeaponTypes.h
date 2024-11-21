#pragma once

#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	//Projectile weapon
	EWT_AssaultRifle UMETA(DisplayName = "Attack Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket"), //NEW1

	//HitScan weapon
	EWT_Pistol UMETA(DisplayName = "Pistol"), //NEW2
	EWT_SMG UMETA(DisplayName = "SMG"), //NEW3
	EWT_Shotgun UMETA(DisplayName = "Shotgun"), //NEW4
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"), //NEW5
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"), //NEW6

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

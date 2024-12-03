// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Blaster/WeaponTypes.h>
#include <Blaster/CharacterTypes.h>
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//We use the same key for them:
	void Equip(class AWeapon* InWeapon);
	void EquipSecondWeaponToBackpack(AWeapon* InWeapon);
	void SwapWeapons();


	void PickupAmmo(EWeaponType InWeaponType, uint32 InAmmoAmmount);


	void AttachEquippedWeaponToRightHandSocket();

	void AttachEquippedWeaponToLeftHandSocket();

	void AttachSecondWeaponToSecondWeaponSocket();

	void ExtractCarriedAmmoFromMap_UpdateHUDAmmos_ReloadIfEmpty();

	void DropCurrentWeaponIfAny();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//stephen name it 'TraceUnderCrosshairs' . In last course we name BoxHit ->better BoxHitResult
	FVector DoLineTrace_UnderCrosshairs(FHitResult& LineHitResult);

	void UpdateHUD_CarriedAmmo();

	void UpdateHUD_CarriedAmmo_SpecializedForShotgun();

	void UpdateHUD_ThrowGrenade();

	void CheckAndSetHUD_CarriedAmmo();

	void CheckAndSetHUD_ThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ReloadEnd();
	//this is specialized for shootgun:
	UFUNCTION(BlueprintCallable)
	void ReloadOneAmmo();

	UFUNCTION(BlueprintCallable)
	void ThrowEnd();

	//stephen didn't have this, he spawn it with Input_Throw LOL, hence must solve 'client part' as well, but me dont need according to universal rule: 'as long as montage is played all devices in the end, any notify will trigger in all devices'
	UFUNCTION(BlueprintCallable)
	void ShowGrenadeMesh();

	UFUNCTION(BlueprintCallable)
	void HideGrenadeMesh_SpawnActualProjectileGrenade();

	UFUNCTION(Server, Reliable)
	void ServerSpawnGrenade(const FVector& Target);

	//UFUNCTION(BlueprintCallable)
	//void EndReload_ContinueFiringIf();

	//Reload
	void Input_Reload();
		//Stephen make it Reliable, but i think this is just for cosmetic UNLESS it involve technical calculation:) 
	UFUNCTION(Server, Reliable)
	void ServerInput_Reload();

	void Input_Throw();
	UFUNCTION(Server, Reliable)
	void ServerInput_Throw();

protected:
	virtual void BeginPlay() override;

private:
//***function***
	//aim
	void SetIsAiming(bool InIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool InIsAiming);

	//fire
	void Input_Fire(bool InIsFiring);

	bool CanFire();

	UFUNCTION(Server, Reliable)
	void ServerInput_Fire(bool InIsFiring, const FVector_NetQuantize& Target);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastInput_Fire(bool InIsFiring, const FVector_NetQuantize& Target);




	void SetPOVForCamera(float DeltaTime);

	//Stephen call it SetHUDCrosshairs
	void SetHUDPackageForHUD(float DeltaTime);

	//Automatic fire:
	void Start_FireTimer();

	void FireTimer_Callback(); //bool InIsFiring, const FVector_NetQuantize& Target

	void Input_Fire_WithoutAssingmentLine();

		float FireDelay=0.25;

		bool bIsAutomatic = true;

		FTimerHandle TimeHandle;

		//FTimerDelegate TimerDelegate;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	void InitializeCarriedAmmo();

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondWeapon();

	UFUNCTION()
	void OnRep_ThrowGrenade();

//***data member***
	
	
	//this no need to be replicated, it is set for all version back in Char::PostInitializeComponents
	UPROPERTY()
	class ABlasterCharacter* Character; //to let this comp aware of its hosting object
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	//to spawn Grenade:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	//need to change it back to Unoccupied via Anim Notify from ABP, BPAccess require not private or need meta = no it is the intermediate one from AimInstance need to be like that lol
	//thi need to be propogated to AnimInstance to be used in ABP
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterState, meta = (AllowPrivateAccess = "true" ))
	ECharacterState CharacterState = ECharacterState::ECS_Unoccupied;
		UFUNCTION()
		void OnRep_CharacterState();

	//the Equipped Pose relying on this to know whether Char has a weapon or not, so that to choose "which group of anims: equipped or not"
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon) //just upgrade it to 'Using' for fixing a client can't change bOrient on itself :D :D
	class AWeapon* EquippedWeapon = nullptr;      //and more

	UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon) 
	class AWeapon* SecondWeapon = nullptr;    

	//UPROPERTY()
	//AWeapon* TempWeapon; //for purpose of swapping weapon

	//FTimerHandle TimerHandle_Swap;
	//UFUNCTION()
	//void TimerCallback_Swap();
	
	UPROPERTY(Replicated)
	bool bIsAiming{};

	//UPROPERTY(Replicated)
	bool bIsFiring{};

	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed_Backup; //backup for initial MaxWalkSpeed, set its value in beginplay, constructor is surely too soon!
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 300.f; //to change MaxWalkSpeed = AimWalkSpeed when we aim

	//UPROPERTY(Replicated)  didn't work
	FVector HitTarget;

	UPROPERTY(EditAnywhere)
	float SphereRadius = 50.f;
	UPROPERTY(EditAnywhere)
	bool bDrawConsistentLine = false;

	//Expand crosshairs:
	UPROPERTY(EditAnywhere)
	float AdditionalJumpFactor{};
	UPROPERTY(EditAnywhere)
	float AdditinalFireFactor{};
	UPROPERTY(EditAnywhere)
	float SubtractiveAimFactor{};

	UPROPERTY(EditAnywhere)
	float JumpFactorMax=2.25f;
	UPROPERTY(EditAnywhere)
	float FireFactorMax=0.65f;
	UPROPERTY(EditAnywhere)
	float AimFactorMax=0.5f;

	//FOV
	float DefaultPOV; //to be set to Character->Camera->POV, it is a backup
	float CurrentPOV;

	//Crosshair color:
	FLinearColor CrosshairsColor;

	UPROPERTY(EditAnywhere)
	float ExtraStartOffset = 10.f; //including D_char/2 + D_gun/2 + Hand_Extent + extraOffset

	bool bCanFire = true;

	//CarriedAmmo for the currently-equipped weapon, hence change weapon will change it too - HUD relevant: create with Char or PS is another option!
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo{}; //let each Char has 45 when GameStart even when they dont have any gun

	UPROPERTY(EditAnywhere)
	uint32 MaxCarriedAmmo = 500;

	//this is for the sake of local organization, not for replication as TMap can't, when we pick a weapon of a specific Weapon::WeaponType (which will be created from there soon) we add it to this map for local organization:
	TMap<EWeaponType, int> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_AR = 45;
	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_Rocket = 15;
	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_Pistol = 20;
	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_SMG = 45;

	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_Shotgun = 8;
	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_SniperRifle = 4;
	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_GrenadeLauncher = 4;

	UPROPERTY(ReplicatedUsing = OnRep_ThrowGrenade)
	int32 ThrowGrenade = 4; //now  = 4 for testing
	UPROPERTY(EditAnywhere)
	int32 ThrowGrenadeCapacity = 4;

public:	
	friend class ABlasterCharacter;     //since already forward-declare, so 'class' here is optional!

	FLinearColor GetCrosshairsColor(){ return CrosshairsColor; }
	void SetCrosshairsColor(const FLinearColor& InColor) { CrosshairsColor = InColor; }

	int32 GetCarriedAmmo() { return CarriedAmmo; }
	bool CanSwapWeapon() { return EquippedWeapon && SecondWeapon ; }
	void DoAction_Fire(bool InIsFiring, const FVector_NetQuantize& Target);
};









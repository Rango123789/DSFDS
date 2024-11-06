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

	void Equip(class AWeapon* InWeapon);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//stephen name it 'TraceUnderCrosshairs' . In last course we name BoxHit ->better BoxHitResult
	FVector DoLineTrace_UnderCrosshairs(FHitResult& LineHitResult);

	void UpdateHUD_CarriedAmmo();

	void CheckAndSetHUD_CarriedAmmo();

	UFUNCTION(BlueprintCallable)
	void EndReload();

	UFUNCTION(BlueprintCallable)
	void EndReload_ContinueFiringIf();

	//Reload
	void Input_Reload();

	//Stephen make it Reliable, but i think this is just for cosmetic UNLESS it involve technical calculation:) 
	UFUNCTION(Server, Reliable)
	void ServerInput_Reload();

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



//***data member***
	
	
	//this no need to be replicated, it is set for all version back in Char::PostInitializeComponents
	UPROPERTY()
	class ABlasterCharacter* Character; //to let this comp aware of its hosting object
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;


	//need to change it back to Unoccupied via Anim Notify from ABP, BPAccess require not private or need meta = no it is the intermediate one from AimInstance need to be like that lol
	//thi need to be propogated to AnimInstance to be used in ABP
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CharacterState, meta = (AllowPrivateAccess = "true" ))
	ECharacterState CharacterState = ECharacterState::ECS_Unoccupied;
		UFUNCTION()
		void OnRep_CharacterState();

	//the Equipped Pose relying on this to know whether Char has a weapon or not, so that to choose "which group of anims: equipped or not"
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon) //just upgrade it to 'Using' for fixing a client can't change bOrient on itself :D :D
	class AWeapon* EquippedWeapon = nullptr;      //and more

	
	UPROPERTY(Replicated)
	bool bIsAiming{};

	//UPROPERTY(Replicated)
	bool bIsFiring{};

	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed_Backup; //backup for initial MaxWalkSpeed, set its value in constructor!
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed; //to change MaxWalkSpeed = AimWalkSpeed when we aim

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

	//this is for the sake of local organization, not for replication as TMap can't, when we pick a weapon of a specific Weapon::WeaponType (which will be created from there soon) we add it to this map for local organization:
	TMap<EWeaponType, int> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartCarriedAmmo_AR = 45;


public:	
	friend class ABlasterCharacter;     //since already forward-declare, so 'class' here is optional!

	FLinearColor GetCrosshairsColor(){ return CrosshairsColor; }
	void SetCrosshairsColor(const FLinearColor& InColor) { CrosshairsColor = InColor; }

	int32 GetCarriedAmmo() { return CarriedAmmo; }
	
};









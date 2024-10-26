// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/CharacterTypes.h"
#include "Interfaces/InteractWithCrossHairsInterface.h"
#include "BlasterCharacter.generated.h" 

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrossHairsInterface
{
	GENERATED_BODY()

public:
/***functions***/
//category1: auto-generated functions:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

//category2: virtual functions:
	/**<Actor>*/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	 /**</Actor>*/

	/**<X>*/
	virtual void Jump() override;
	 /**</X>*/

//category3: callbacks and RPC
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable) //this is important event, so we make it Reliable as well
	void ServerEKeyPressed();

	//UFUNCTION(NetMulticast , Unreliable)
	//void MulticastPlayHitReactMontage(); //no need any more

	virtual void OnRep_ReplicatedMovement() override;

	//this time we create callback to be bound to OnTakeAnyDamage, rather than override AActor::TakeDamage
	UFUNCTION() //because OnTakeAnyDamgage is DYNAMIC delegate
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

//category4: regular functions: 
	//montages:
	void PlayMontage_SpecificSection(UAnimMontage* InMontage, FName InName = "");
	void PlayFireMontage();
	void StopFireMontage();

	void PlayHitReactMontage();
	//sound and effects:

	//bool functions:
	bool IsWeaponEquipped();
	bool IsAming();
	bool IsAFiring();

	//BP-callale functions:
	
	//others:
	//void SetIsAiming(bool InIsAiming);	//REPLACE
	void SetupAimOffsetVariables(float DeltaTime);

	void TurnInPlace_ForAutoProxyOnly(float DeltaTime);

	void Turn_ForSimProxyOnly();

	void Elim();

protected:
	/***functions***/
//category1: auto-generated functions:
	virtual void BeginPlay() override;
//category2: virtual functions:
	/**<Actor>*/

	 /**</Actor>*/

	/**<X>*/

	 /**</X>*/

//category3: regular functions 

//category4: callbacks

//category5: replication


/***data members****/
//Category1: Enums , arrays, pointers to external classes
	//enum states:
	ETurningInPlace TurningInPlace = ETurningInPlace::RTIP_NoTurning;
	//pointer to external classes:
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) // OnRep_[ReplicatedMember]() 
	class AWeapon* OverlappingWeapon;

	class ABlasterPlayerController* PlayerController; //NEWs

	//	//HUD and its Overlay widget
	//class ABlasterHUD* BlasterHUD;
	//class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget;

	//arrays:

	//class type:

//category2: UActorComponents   
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm; //CameraBoom
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;       //FollowCamera 

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* Overhead_WidgetComponent;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

//category3: Engine types      
	//montages:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_FireMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_HitReact;

	//sound and effects:

//category4: basic and primitive types
	//UPROPERTY(EditAnywhere) //REPLACE
	//float MaxWalkSpeed_Backup; //backup for initial MaxWalkSpeed, set its value in constructor!
	//UPROPERTY(EditAnywhere)
	//float AimWalkSpeed; //REPLACE

	float AO_Yaw; //DeltaYawSinceStopMovingJumping
	float AO_Pitch; //the Pitch of ControlRotation<-Camera

	float AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero;

	FRotator BaseAimRotation_SinceStopMoving;

	UPROPERTY(EditAnywhere)
	float InterpSpeed_Turning = 10.5f;

	UPROPERTY(EditAnywhere)
	float X_stop = 1.f;

	bool bShouldRotateRootBone{};

	UPROPERTY(EditAnywhere)
	float TurnThreshold = 0.5 ; // /60.f
	float ProxyDeltaYaw{};

	FRotator ProxyRotation;
	FRotator ProxyRotation_LastFrame;

	float AccumilatingTime;

	UPROPERTY(EditAnywhere)
	float TimeThreshold;

private: 
	/***functions***/
//category1: auto-generated functions:

//category2: virtual functions:

//category3: callbacks
	void Input_Move(const struct FInputActionValue & Value); //in UE5.2 need to forward-declare this struct
	void Input_Look(const FInputActionValue & Value);
	void Input_Jump(const FInputActionValue & Value);
	void Input_EKeyPressed(const FInputActionValue& Value);
	void Input_Crouch(const FInputActionValue& Value);
	void Input_Aim_Pressed(const FInputActionValue& Value);
	void Input_Aim_Released(const FInputActionValue& Value);
	void Input_Fire_Pressed(const FInputActionValue& Value);
	void Input_Fire_Released(const FInputActionValue& Value);

//category4: regular functions 
	void HideCharacterIfCameraClose();



/***data members****/
//Category1: Enums , arrays, pointers to external classes

//category2: UActorComponents   

//category3: Engine types      
    //input:
	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_Blaster;

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Move;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Look;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Jump;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_EKeyPressed;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Crouch;

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Aim_Pressed;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Aim_Released;

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Fire_Pressed;
	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Fire_Released;



	//montages:

	//sound and effects:

//category4: basic and primitive types
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 20.f;

	//attributes:
	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere) //EditAnywhere, for debugging purpose
	float Health = 10.f;

	UFUNCTION()
	void OnRep_Health();

public:	
	/***Setters and Getters***/

	void SetOverlappingWeapon(AWeapon* InWeapon);

	float GetAO_Yaw() { return AO_Yaw; }
	float GetAO_Pitch() { return AO_Pitch; }

	UCombatComponent* GetCombatComponent() { return CombatComponent; }

	AWeapon* GetEquippedWeapon();

	void SetTurningInPlace(ETurningInPlace InValue) { TurningInPlace = InValue; }

	ETurningInPlace GetTurningInPlace() { return TurningInPlace; }

	UCameraComponent* GetCamera() { return Camera; }

	bool GetShouldRotateRootBone() { return bShouldRotateRootBone; }
};

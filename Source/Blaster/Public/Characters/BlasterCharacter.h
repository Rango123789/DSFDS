// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
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

	 /**</X>*/


//category3: callbacks and RPC
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable) //this is important event, so we make it Reliable as well
	void ServerEKeyPressed();

	UFUNCTION(Server, Reliable) 
	void ServerSetIsAiming(bool InIsAiming);

//category4: regular functions: 
	//montages:

	//sound and effects:

	//bool functions:
	bool IsWeaponEquipped();
	bool IsAming();

	//BP-callale functions:
	
	//others:
	void SetIsAiming(bool InIsAiming);	
	void SetupAimOffsetVariables(float DeltaTime);

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

	//pointer to external classes:
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) // OnRep_[ReplicatedMember]() 
	class AWeapon* OverlappingWeapon;

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

	//sound and effects:

//category4: basic and primitive types
	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed_Backup; //backup for initial MaxWalkSpeed, set its value in constructor!
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed; //to change MaxWalkSpeed = AimWalkSpeed when we aim

	float AO_Yaw; //DeltaYawSinceStopMovingJumping
	float AO_Pitch; //the Pitch of ControlRotation<-Camera

	FRotator BaseAimRotation_SinceStopMoving;

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

//category4: regular functions 
	


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

	//montages:

	//sound and effects:

//category4: basic and primitive types

public:	
	/***Setters and Getters***/

	void SetOverlappingWeapon(AWeapon* InWeapon);

	float GetAO_Yaw() { return AO_Yaw; }
	float GetAO_Pitch() { return AO_Pitch; }

	//UFUNCTION(BlueprintCallable)
	//void SetWeaponPickWidgetVisibility(bool bIsVisible = true);

};

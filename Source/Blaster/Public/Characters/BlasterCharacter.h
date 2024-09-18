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
	 /**</Actor>*/

	/**<X>*/

	 /**</X>*/

//category3: regular functions: 
	//montages:

	//sound and effects:

	//bool functions:

	//BP-callale functions:
	
//category4: callbacks 

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* UpdatedWeapon);

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

/***data members****/
//Category1: Enums , arrays, pointers to external classes
	//enum states:

	//pointer to external classes:
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) // OnRep_[ReplicatedMember]() 
	class AWeapon* OverlappingWeapon;

	UPROPERTY(Replicated)
	AWeapon* LastOverlappingWeapon;

	//arrays:

	//class type:

//category2: UActorComponents   
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm; //CameraBoom
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;       //FollowCamera 

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* Overhead_WidgetComponent;


//category3: Engine types      
	//montages:

	//sound and effects:

//category4: basic and primitive types



private: 
	/***functions***/
//category1: auto-generated functions:

//category2: virtual functions:

//category3: regular functions 

//category4: callbacks
	void Input_Move(const struct FInputActionValue & Value); //in UE5.2 need to forward-declare this struct
	void Input_Look(const FInputActionValue & Value);
	void Input_Jump(const FInputActionValue & Value);

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
	//montages:

	//sound and effects:

//category4: basic and primitive types

public:	
	/***Setters and Getters***/

	void SetOverlappingWeapon(AWeapon* InWeapon);

	//UFUNCTION(BlueprintCallable)
	//void SetWeaponPickWidgetVisibility(bool bIsVisible = true);

};

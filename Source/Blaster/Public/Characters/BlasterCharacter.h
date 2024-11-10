// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/CharacterTypes.h"
#include "Interfaces/InteractWithCrossHairsInterface.h"
#include "Components/TimelineComponent.h" //have to put it here for FOnTimeline
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

	virtual void OnRep_ReplicatedMovement() override;

	//this time we create callback to be bound to OnTakeAnyDamage, rather than override AActor::TakeDamage
	UFUNCTION() //because OnTakeAnyDamgage is DYNAMIC delegate
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void UpdateHUD_Health();

//category4: regular functions: 
	//montages:
	void PlayMontage_SpecificSection(UAnimMontage* InMontage, FName InName = "Default");
	void PlayFireMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReloadMontage();
	//sound and effects:

	//bool functions:
	bool IsWeaponEquipped();
	bool IsAming();
	bool IsAFiring();

	//BP-callale functions:
	UFUNCTION(BlueprintCallable)
	void ResetCharacterStateToUnoccupied();

	UFUNCTION(BlueprintCallable)
	void ReloadEnd1();


	//others:
	//void SetIsAiming(bool InIsAiming);	//REPLACE
	void SetupAimOffsetVariables(float DeltaTime);

	void TurnInPlace_ForAutoProxyOnly(float DeltaTime);

	void Turn_ForSimProxyOnly();

	//this is just an option of GOLDEN0, you can even call it directly in ReceiveDamage, and then paste into OnRep_Health() as well
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void Elim();
	
	void SetupEnhancedInput_IMC();
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
    // poll (observe) for any relevant classes and initalize our HUD:
	void PollInit();
//category4: callbacks

//category5: replication


/***data members****/
//Category1: Enums , arrays, pointers to external classes
	//enum states:
	ETurningInPlace TurningInPlace = ETurningInPlace::RTIP_NoTurning;

	//pointer to external classes:
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) // OnRep_[ReplicatedMember]() 
	class AWeapon* OverlappingWeapon;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController; //NEWs

	UPROPERTY()
	class APlayerState_Blaster* PlayerState_Blaster; //NEWs

	//not sure it is a good idea to create a member of this where this is only meaningful to the server device
	//class ABlasterGameMode* BlasterGameMode;

	//arrays:

	//class type:

//category2: UActorComponents   
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm; //CameraBoom
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;       //FollowCamera 

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* Overhead_WidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
	class UCombatComponent* CombatComponent;

	//for Timeline: (I move them apart later)
	UPROPERTY(VisibleAnywhere)
	class UTimelineComponent* TimelineComponent; //this is UActorComp ->need CreateDefaultSubobject<T>
	
	UPROPERTY(EditAnywhere)
	class UCurveFloat* CurveFloat_Dissolve;
	//we need this
		//UPROPERTY(EditAnywhere)
		//FTimelineFloatTrack TimelineFloatTrack_Dissolve; //dont need this so far, in turn has sub member 'UCurveFloat*'
	FOnTimelineFloat OnTimelineFloat_Delegate_Dissolve; //this is DYNAMIC delegate, with 'float' signature

	void StartTimeline_Dissolve(); //OPTIONAL, but highly recommended, StartTimeline will prepare 2+ lines!	

	UFUNCTION()
	void OnTimelineFloat_Callback_Dissolve(float DissolveAdding);

	//Ready Material for Timeline_callback (where external things work on top of DYNAMIC value of Timeline's curve)
	UPROPERTY(EditAnywhere) 
	UMaterialInstance* MaterialInstance; //to be picked from BP

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* MaterialInstanceDynamic; // to work on top of 'MaterialInstance' and to be DIRECTLY assigned for GetMesh() through out Timeline

	//OPTIONAL: In case you want to change GetMesh() from non-optimized to optimized as Elim reach
	UPROPERTY(EditAnywhere)
	USkeletalMesh* SkeletalMesh_Optimized;

//category3: Engine types      
	//montages:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_FireMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_HitReact;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_ElimMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_ReloadMontage;

	//sound and effects:
	UPROPERTY(EditAnywhere) 
	UParticleSystem* BotParticle; //(*)
	
	//We dont intent to create default subobject of this, we just store the object return by (*) in MulticastElim, for a weird reason: it can't destroy itself after finshing playing :D :D. 
	//Not sure it has been change in UE5.2 but let's see.
	//UPDATE: no need anymore it auto-destroy itself in UE5.2+ :D :D, create a member storing it only lengthne its timelife stupidly :D :D
	//UParticleSystemComponent* ParticleSystemComponent; 

	UPROPERTY(EditAnywhere)
	USoundBase* BotSound;

	//Timer:
		//the character wont be in the world at first for you to EditInstance, also we would have different elim for different elim delay per character, it is not fair, so yeah that's why Stephen Play this tokent!
	UPROPERTY(EditDefaultsOnly) 
	float DelayTime_Elim = 3.f;

	FTimerHandle TimerHandle_Elim;
	UFUNCTION()
	void TimerCallback_Elim();


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

	bool bIsEliminated = false; //IsFiring and IsAimin is in CombatComponent, not here

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

	void Input_Reload(const FInputActionValue& Value);

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

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Reload;

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

	bool GetIsEliminated() { return bIsEliminated; }

	float GetHealth() { return Health; }
	float GetMaxHealth() { return MaxHealth; }

	ABlasterPlayerController* GetBlasterPlayerController();

	ECharacterState GetCharacterState(); 

};

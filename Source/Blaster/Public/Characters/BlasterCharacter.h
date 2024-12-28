// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/CharacterTypes.h"
#include "Interfaces/InteractWithCrossHairsInterface.h"
#include "Components/TimelineComponent.h" //have to put it here for FOnTimeline
#include "BlasterCharacter.generated.h" 

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSendingDestroySessionRequestDelegate_Char);

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrossHairsInterface
{
	GENERATED_BODY()

public:
/***functions***/
//category1: auto-generated functions:
	ABlasterCharacter(); 
	virtual void Tick(float DeltaTime) override;
	void AimOffsetAndTurnInPlace_GLOBAL(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

//category2: virtual functions:
	/**<Actor>*/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;
	//alternative for PC::OnPossess() medicin
	virtual void PossessedBy(AController* NewController) override;
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

	void CheckAndUpdateHUD_Health();
	void CheckAndUpdateHUD_Shield();

//category4: regular functions: 
	//montages:
	void PlayMontage_SpecificSection(UAnimMontage* InMontage, FName InName = "Default");
	void PlayFireMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayThrowMontage();
	void PlaySwapMontage();
	void JumpToShotgunEndSection();
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

	UFUNCTION(BlueprintImplementableEvent)
	void ShowScopeWidget(bool bUseScopeWidget);

	//others:
	//void SetIsAiming(bool InIsAiming);	//REPLACE
	void SetupAimOffsetVariables(float DeltaTime);

	void TurnInPlace_ForAutoProxyOnly(float DeltaTime);

	void Turn_ForSimProxyOnly();

	//this is just an option of GOLDEN0, you can even call it directly in ReceiveDamage, and then paste into OnRep_Health() as well
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bInPlayerLeavingGame);

	void Elim(bool bInPlayerLeavingGame);
	
	void SetupEnhancedInput_IMC();

	void ShowGrenadeMesh();
	void HideGrenadeMesh();

	//call this in Char::BeginPlay(), checking if current gamemode is BlasterGameMode before spawn it:
	void SpawnDefaultWeapon();

	//exceptional public DATA:
	UPROPERTY()
	TMap<FName, class UBoxComponent*> BoxComponentMap;

	bool bIsLocalSwapping = false; 

	UFUNCTION(Server, Reliable)
	void ServerLeaveGameRequest();

	//will be set value in MulticastElim() so dont need to be marked Replicated:
	bool bPlayerLeavingGame = false;

	FOnSendingDestroySessionRequestDelegate_Char OnSendingDestroySessionRequestDelegate_Char;

	//Show crown: you can create a single MulticastRPC with bool param, but stephen choose to create 2 separates, so that RPC dont need to send any DATA for paramter part:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowCrown();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRemoveCrown();

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
	class ABlasterHUD* BlasterHUD = nullptr;

	UPROPERTY()
	class UCharacterOverlay_UserWidget* CharacterOverlay_UserWidget = nullptr;

	UPROPERTY()
	class APlayerState_Blaster* PlayerState_Blaster; //NEWs

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;//NEWs

	//not sure it is a good idea to create a member of this where this is only meaningful to the server device
	//class ABlasterGameMode* BlasterGameMode;

	//arrays:

	//class type:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> DefaultWeaponClass;

//category2: UActorComponents   
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm; //CameraBoom
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;       //FollowCamera 

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* Overhead_WidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ULagCompensationComponent* LagComponent = nullptr;

	//boxes for server-rewind technique:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* head;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* spine_03;


	UPROPERTY(VisibleAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* blanket_l;


	UPROPERTY(VisibleAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hand_l;


	UPROPERTY(VisibleAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* hand_r;


	UPROPERTY(VisibleAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* foot_l;


	UPROPERTY(VisibleAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* foot_r;

	//temp:
	UPROPERTY()
	class UNiagaraComponent* NiagaraComponent_Crown;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* NS_Crown;

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

	/*
	*Materials
	*/
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot0_NonOptimizedCharacter; //to be picked from BP
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot0_NonOptimizedCharacter_Red; //to be picked from BP
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot0_NonOptimizedCharacter_Blue; //to be picked from BP

	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot1_NonOptimizedCharacter; //to be picked from BP
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot1_NonOptimizedCharacter_Red; //to be picked from BP
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Slot1_NonOptimizedCharacter_Blue; //to be picked from BP

	//MI_Disspolve
	UPROPERTY(EditAnywhere) 
	UMaterialInstance* MI_Dissolve_ForOptimizedCharacter; //to be picked from BP
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Dissolve_ForOptimizedCharacter_Red; //to be picked from BP if a player is RedTeam
	UPROPERTY(EditAnywhere)
	UMaterialInstance* MI_Dissolve_ForOptimizedCharacter_Blue; //to be picked from BP if a player is BlueTeam

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* MaterialInstanceDynamic; //to work on top of 'MaterialInstance' and to be DIRECTLY assigned for GetMesh() through out Timeline.

	//OPTIONAL: In case you want to change GetMesh() from non-optimized to optimized as Elim reach
	UPROPERTY(EditAnywhere)
	USkeletalMesh* SkeletalMesh_Optimized;

	//TempGrenade for stage1 of throwing:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* TempGrenadeMesh;

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

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_ThrowMontage;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* AM_SwapMontage;

	//sound and effects:
	UPROPERTY(EditAnywhere) 
	UParticleSystem* BotParticle; //(*)
	  //stephen also create this, and then call ->Destroy() in char::Destroyed(), BUT i THINK it may not needed when you choose 'bAutoDestroy = true' as you spawn the BotParticle; as well well make sure the asset has no part last longer than it should:
	UPROPERTY()
	UParticleSystemComponent* BotParticleComp;


	
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

	UPROPERTY(Replicated)
	bool bDisableMostInput = false;

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
	void Input_Throw(const FInputActionValue& Value);
	//void Input_ReturnToMainMenu(const FInputActionValue& Value);


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

	UPROPERTY(EditAnywhere)
	class UInputAction* IA_Throw;

	//UPROPERTY(EditAnywhere)
	//class UInputAction* IA_ReturnToMainMenu;

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

	//add more parameter to fix the bug
	UFUNCTION()
	void OnRep_Health(float Health_LastFrame);

	//attributes:
	UPROPERTY(EditAnywhere)
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere) //EditAnywhere, for debugging purpose
	float Shield = 10.f;

	//add more parameter to fix the bug
	UFUNCTION()
	void OnRep_Shield(float Shield_LastFrame);

public:	
	/***Setters and Getters***/

	void SetOverlappingWeapon(AWeapon* InWeapon);

	float GetAO_Yaw() { return AO_Yaw; }
	float GetAO_Pitch() { return AO_Pitch; }

	UCombatComponent* GetCombatComponent() { return CombatComponent; }
	UBuffComponent* GetBuffComponent() { return BuffComponent; }

	AWeapon* GetEquippedWeapon();

	void SetTurningInPlace(ETurningInPlace InValue) { TurningInPlace = InValue; }

	ETurningInPlace GetTurningInPlace() { return TurningInPlace; }

	UCameraComponent* GetCamera() { return Camera; }

	bool GetShouldRotateRootBone() { return bShouldRotateRootBone; }

	bool GetIsEliminated() { return bIsEliminated; }

	float GetHealth() { return Health; }
	float GetMaxHealth() { return MaxHealth; }
	void SetHealth(float InHealth) { Health = InHealth; };
	void AddHealth(float InExtraHealth) { Health += InExtraHealth; };

	float GetShield() { return Shield; }
	float GetMaxShield() { return MaxShield; }
	void SetShield(float InShield) { Shield = InShield; };

	ABlasterPlayerController* GetBlasterPlayerController();

	ECharacterState GetCharacterState(); 

	bool GetDisableMostInput() { return bDisableMostInput; }

	void SetDisableMostInput(bool InBool) { bDisableMostInput = InBool; }

	bool GetIsElimminated() { return bIsEliminated; };
	bool GetIsLocalReloading();
	//bool GetIsLocalSwapping();
	class ULagCompensationComponent* GetLagComponent() { return LagComponent; }
};
 
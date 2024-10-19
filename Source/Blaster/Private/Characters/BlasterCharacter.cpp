// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterCharacter.h"
#include "CharacterComponents/CombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "HUD/Overhead_UserWidget.h"
#include "Weapons/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"//test
#include "Blaster/Blaster.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; //it is true by default, no need
	//Setup SpringArm and Camera: 
	  //normally we attach SpringArm too 'RootComponent' (which is capsule here), but LATER ON we will use 'Crouch' function - that we need to scale the capsule/root component DOWN, so if a parent is scaled its attached childs will scaled too, including relative location to the parent LOL, hence the location of SpringArm to its parent change ABNORMALLY, we dont want it so perhaps GetMesh() is the choice here
	  //But scale Capsule/RootComponent will also Scale GetMesh() and hence scale SpringArm as well LOL, so what next to solve this? well we'll see dont worry
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600; //the default is 300
	SpringArm->bUsePawnControlRotation = true; //the default is surprisingly 'false' LOL

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); //the Socket name is optional I think, whether or not you specify it it will au-to attacj to that 'SocketName' by default behaviour LOL
	Camera->bUsePawnControlRotation = false; //optional because it already attach to SpringArm that is set to use it already above, but if not do this 2 same code 'MAY' run twice in the same frame, not good.

	//Setup UWidgetComponent:
	Overhead_WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead_WidgetComponent"));
	Overhead_WidgetComponent->SetupAttachment(RootComponent);

	//Setup UCombatComponent: We will replicate this Component[/pointer object] ; it is not SceneComponent so...
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true); // || ->SetIsReplicatedByDefault(true) || ->bReplicates = true || directly set it from the local class AWeapon is also fine - I refer to do it where the local class is LOL. But it may set back false if it is within the other class?, It could be LOL.

	//make sure you can
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);

	//GetMesh()->SetNotifyRigidBodyCollision(true); //no need, only need to check where Hit Event need to occur
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Create UEnhancedInputLocalPlayerSubsystem_object associate with ULocalPlayer+APlayerController controlling this pawn:
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController && IMC_Blaster)
	{
		//create __ object and associate it with the LocalPlayer object, hence PlayerController, currently controlling this Character: 
		UEnhancedInputLocalPlayerSubsystem* EISubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		//the priority=0 is just for fun, there is only one IMC we're gonna add to our Blaster Character: 
		if(EISubsystem) EISubsystem->AddMappingContext(IMC_Blaster, 0);
	}

	//call the helper function to change text of the underlying widget of the widget component:
	if (Overhead_WidgetComponent)
	{
		UOverhead_UserWidget* Overhead_UserWidget = Cast<UOverhead_UserWidget>(Overhead_WidgetComponent->GetUserWidgetObject());
		if (Overhead_UserWidget) Overhead_UserWidget->ShowPlayerNetRole(this);
	}

	BaseAimRotation_SinceStopMoving = GetBaseAimRotation();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//OnRep_ can't be called on server so it will work for all clients, except the server
	// hence need additional work for the server separately, independent from this replication LOL
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon , COND_OwnerOnly);
	//DOREPLIFETIME_CONDITION(ABlasterCharacter, LastOverlappingWeapon, COND_OwnerOnly);

	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
}

void ABlasterCharacter::PostInitializeComponents()
{
	//supposedly all components should be initialized and not null before this Post, but still I want to check as good practice:
	Super::PostInitializeComponents();
	if (CombatComponent) CombatComponent->Character = this;
}

//use 'else' if you want press TWICE to jump, dont use 'else' if you want press ONCE to jump from Crouch state - no , either case lead to the same end: press TWICE to jump.
void ABlasterCharacter::Jump()
{
	if (bIsCrouched) UnCrouch();
	else Super::Jump();
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	SetupAimOffsetVariables(DeltaTime);
	HideCharacterIfCameraClose();
}

//return true if EquippedWeapon is NOT null
bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAming()
{
	return (CombatComponent && CombatComponent->bIsAiming);
}

bool ABlasterCharacter::IsAFiring()
{
	return (CombatComponent && CombatComponent->bIsFiring);
}

//this can't only be called on client copies: adding param/using it or not doesn't matter
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	//Reacheaching this OnRep boby the OverlappingWeapon must be assigned new value already
	//hence either of them will be executed, but not both I'm sure :D :D
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	else if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//this will be called in Weapon::Overlap which could only happen in-server copy
//hence MANUAL work condition can count on it!
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* InWeapon)
{
	//before the time we assign it a new value, check if it is not null
	//then should turn it OFF, before it receive new value that is surely null :D :D
	//if it is nullptr, then it will pass this line, without turning it off it is surely already OFF from last time LOL
	//if it is nullptr, it will pass this line and to be turn ON in the final block!
	if (OverlappingWeapon ) //&& IsLocallyControlled() , &&InWeapon == nullptr - no need
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = InWeapon;

    //if it is assigned new value, but it is not null, then I think we have to turn it ON
	//if it is assigned new value, but it is null, then it will pass this line, it is OFF by first block above, which is exactly what we want.
	if (OverlappingWeapon &&  IsLocallyControlled())
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	else return CombatComponent->EquippedWeapon;
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedPlayerInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedPlayerInputComponent)
	{
 		EnhancedPlayerInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move );
		EnhancedPlayerInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
		EnhancedPlayerInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_Jump);
		EnhancedPlayerInputComponent->BindAction(IA_EKeyPressed, ETriggerEvent::Triggered, this, &ThisClass::Input_EKeyPressed);
		EnhancedPlayerInputComponent->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch);

		EnhancedPlayerInputComponent->BindAction(IA_Aim_Pressed, ETriggerEvent::Triggered, this, &ThisClass::Input_Aim_Pressed);
		EnhancedPlayerInputComponent->BindAction(IA_Aim_Released, ETriggerEvent::Triggered, this, &ThisClass::Input_Aim_Released);

		EnhancedPlayerInputComponent->BindAction(IA_Fire_Pressed, ETriggerEvent::Triggered, this, &ThisClass::Input_Fire_Pressed);
		EnhancedPlayerInputComponent->BindAction(IA_Fire_Released, ETriggerEvent::Triggered, this, &ThisClass::Input_Fire_Released);
	}
}

void ABlasterCharacter::MulticastPlayHitReactMontage_Implementation()
{
	PlayHitReactMontage();
}

void ABlasterCharacter::PlayMontage_SpecificSection(UAnimMontage* InMontage, FName InName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && InMontage)
	{
		AnimInstance->Montage_Play(InMontage);

		AnimInstance->Montage_JumpToSection(InName, InMontage);
	}
}

void ABlasterCharacter::PlayFireMontage()
{
	if (CombatComponent == nullptr || AM_FireMontage == nullptr) return;

	//from AM_ asset from BP, you must name its sections to match these below:
	FName SectionName = CombatComponent->bIsAiming ? FName("RifleAim") : FName("RifleHip");

	PlayMontage_SpecificSection(AM_FireMontage, SectionName);
}

void ABlasterCharacter::StopFireMontage()
{
	if (AM_FireMontage == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance) AnimInstance->Montage_Stop(0.2f, AM_FireMontage);

	//AnimInstance->PlaySlotAnimationAsDynamicMontage();
	//GetMesh()->PlayAnimation();
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(AM_FireMontage == nullptr) return;

	FName SectionName("FromFront"); //for now

	PlayMontage_SpecificSection(AM_HitReact, SectionName);

}

void ABlasterCharacter::Input_Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (!Controller || Input.Size() == 0) return; //Input.Size() == 0 improve performance, where Controller are optional I think

	//FVector direction1 = GetActorRightVector(); - this move in curent Actor direcion, not Camera direction
        //FVector direction2 = GetActorForwardVector();

	//the key to solve confustion is: Value.X match Right vector, Value.Y match Forward vector:
	FVector direction1 = FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y); //right vector, but match X of value
	FVector direction2 = FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X); //forware vector, match Y of value

	//FVector direction1 = UKismetMathLibrary::GetRightVector (GetControlRotation() ) - also work
	//FVector direction2 = UKismetMathLibrary::GetForwardVector (GetControlRotation() )

	//FVector direction1 = SpringArm->GetRightVector; - also work
	//FVector direction2 = SpringArm->GetForwardVector; 

	//FVector direction1 = Camera->GetRightVector; - also work
	//FVector direction2 = Camera->GetForwardVector; 

	//for movement: align with Camera direction
	AddMovementInput(direction1, Input.X);
	AddMovementInput(direction2, Input.Y);

}

void ABlasterCharacter::Input_Look(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (!Controller || Input.Size() == 0) return;

	//yaw to turn around horizontally, pitch to turn up and down; you dont want to 'roll' LOL: 
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void ABlasterCharacter::Input_Jump(const FInputActionValue& Value)
{
	//Super::Jump(); //you just override it, so call the new version instead

	Jump();
}

void ABlasterCharacter::Input_EKeyPressed(const FInputActionValue& Value)
{
	if (CombatComponent == nullptr || OverlappingWeapon == nullptr) return;
	//Without "HasAuthority()" both server add clients can pick. Server pick, clients see it. 
	// where clients pick the server dont see it - but WHY? 
	// where one client pick, other client dont see it as well - by WHY?
	// Because the COLLISION in clients doesn't exist? well but they has reference to OverlappingWeapon decently right? 
	// Because replication only from Server to Clients, or because ReplicatedUsing?
	// Because CombatComponent->SetIsReplicated() indirectly here in this hosting class? But We didn't mark CombatComponent with 'Replicated' from hosting class perspective yet right? So this is NOT yet relevant! it is only self-replicated so far.
	//With "Hasauthority()" only server can pick, clients see it - but HOW? because "Aweapon && Char::OverlappingWeapon" are set to replicated? 
	// where Clients can't even pick - make sense
	if (HasAuthority())
	{
		CombatComponent->Equip(OverlappingWeapon); // (*)
	}
	else
	{
		ServerEKeyPressed(); //its body is purposely indeptical with (*), for clear reason 
	}
}

void ABlasterCharacter::ServerEKeyPressed_Implementation()
{
	if (CombatComponent) CombatComponent->Equip(OverlappingWeapon);
}

void ABlasterCharacter::Input_Crouch(const FInputActionValue& Value)
{
	if (!bIsCrouched) Crouch();
	else UnCrouch();
}


void ABlasterCharacter::Input_Aim_Pressed(const FInputActionValue& Value)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->SetIsAiming(true);
}

void ABlasterCharacter::Input_Aim_Released(const FInputActionValue& Value)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->SetIsAiming(false);
}

void ABlasterCharacter::Input_Fire_Pressed(const FInputActionValue& Value)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->Input_Fire(true);
}

void ABlasterCharacter::Input_Fire_Released(const FInputActionValue& Value)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->Input_Fire(false);
}

void ABlasterCharacter::HideCharacterIfCameraClose()
{
	//We must not hide in other machines, as the still need to see you LOL
	if (IsLocallyControlled() == false) return;
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || 
		CombatComponent->EquippedWeapon->GetWeaponMesh() == nullptr) return;

	float Distance = (GetActorLocation() - Camera->GetComponentLocation()).Size();
	
	if (Distance < CameraThreshold)
	{
	//For Chararacter itself:
		//SetActorHiddenInGame(true);    //Replicated, do de-effect IsLocallyControlled(), NOT wanted
		GetMesh()->SetVisibility(false); // NOT Replicated, hence we need it here. 
	//For EquippedWeapon:
		//CombatComponent->EquippedWeapon->SetActorHiddenInGame(true); //Replicated, do de-effect IsLocallyControlled(), NOT wanted
		//CombatComponent->EquippedWeapon->GetWeaponMesh()->SetVisibility(false); // NOT Replicated, hence we need it here. 
		CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;   // WAY2 for WeaponMesh, only work when you did set the charater is onwer of this, and this will affect the contorlling device ONLY
	}
	else
	{
		//SetActorHiddenInGame(false); // || GetMesh()->SetVisibility(true)
		GetMesh()->SetVisibility(true);
		//CombatComponent->EquippedWeapon->SetActorHiddenInGame(true);
		//CombatComponent->EquippedWeapon->GetWeaponMesh()->SetVisibility(true);
		CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
	}
}

////Stephen create this in UActorComponent instead.
//void ABlasterCharacter::SetIsAiming(bool InIsAiming) //REPLACE
//{
//	if (CombatComponent == nullptr) return;
//
//	if (HasAuthority())
//	{
//		CombatComponent->bIsAiming = InIsAiming;
//		if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = InIsAiming ? AimWalkSpeed : MaxWalkSpeed_Backup;
//	}
//	else
//		ServerSetIsAiming(InIsAiming);
//}


//void ABlasterCharacter::ServerSetIsAiming_Implementation(bool InIsAiming) //REPLACE
//{
//	if (CombatComponent)
//	{
//		CombatComponent->bIsAiming = InIsAiming;
//		if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = InIsAiming ? AimWalkSpeed : MaxWalkSpeed_Backup;
//	}
//}

void ABlasterCharacter::SetupAimOffsetVariables(float DeltaTime)
{
	//UPDATE: this setup is meant for when you have a weapon, and you can skip this if you didn't have any
	//this does improve performance, so why not:
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)  return;
    
	//these lines are just for readibility:
	float GroundSpeed = GetVelocity().Size2D();
	bool IsInAir = GetCharacterMovement()->IsFalling();

	//BLOCK0: GLOBAL else if about "moving/jumping" or not
	if (GroundSpeed == 0.f && !IsInAir  ) //not moving, not jumping
	{
		bUseControllerRotationYaw = true; //from false in last lesson

		FRotator DeltaRotation = (GetBaseAimRotation() - BaseAimRotation_SinceStopMoving);

		AO_Yaw = DeltaRotation.GetNormalized().Yaw;

    //BLOCK1,2 is for "Turning In Place" feature, Without them, AO_Yaw and AimOffset will still work (but remember to set bUseYaw = false back LOL.

		//NEW BLOCK1: this is impossible to stop moving and then rotate beyond 90 right away, so it makes sense to check do this "local else if" first
		if (TurningInPlace == ETurningInPlace::RTIP_NoTurning)
		{
			//NO NEED ? we need it during re-set AO_YAW process, because AO_Yaw is also assgined value in the outer block (which always happen before this block), hence at the end of the process we also need to re-set BaseAimRotation_SinceStopMoving = GetBaseAimRotation() too!
			AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero = AO_Yaw; 
		}
		else //either TurnLeft or TurnRight
		{
			AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero = FMath::FInterpTo( AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero, 0.f, DeltaTime, InterpSpeed_Turning); //NO NEED ? -> need during turn process

			AO_Yaw = AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero; //NO NEED ? -> need during turn process
			
			//it is up to up to decide when to stop interpolate, x = 15 -->stop short at 90-15 = 75 degree, accepting the skip 15 degree. 
			//this local Block is absolutely important so that you can escape either TurnLeft or TurnRight naturally and automatically, otherwise no way to escape it naturally even when the AO_Yaw_SinceNoTurning reach ZERO in interpolation, unless you move again to escape it :D :D
			if (abs(AO_Yaw) <= X_stop) //Stephen < 15.f, I update this!
			{
				TurningInPlace = ETurningInPlace::RTIP_NoTurning;

				BaseAimRotation_SinceStopMoving = GetBaseAimRotation(); //we can't forget this
			}
		}

		//NEW BLOCK2: this is for animation, and also trigger "else" of BLOCK1
		if (AO_Yaw > 90.f)
		{
			SetTurningInPlace(ETurningInPlace::RTIP_TurnRight);
		}
		if (AO_Yaw < -90.f)
		{
			SetTurningInPlace(ETurningInPlace::RTIP_TurnLeft);
		}
	}
	else //running or jumping - this is "GLOBAL else", hence when you move+ "all AimOffset->TurningInPlace" effect will be removed
	{
		bUseControllerRotationYaw = true; //already true from last lesson ->now you can remove both lines

		BaseAimRotation_SinceStopMoving = GetBaseAimRotation();
		
		AO_Yaw = 0; 

		SetTurningInPlace(ETurningInPlace::RTIP_NoTurning); //This is another way to stop the TurningInPlace process, if happening.
	}

	//add .GetNormalized() fix it!
	AO_Pitch = GetBaseAimRotation().GetNormalized().Pitch; 

	UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw); 
}

/* Seperate SetAimOffsetVars() from SetTurnInPlaceVars()
-----------------------
1. Tick()
{
   Super::Tick(DeltaTime);
   SetAimOffsetVariables(DeltaTime);
   SetTurnInPlaceVariables(DeltaTime);
}

@@NOTE:
+'SetTurnInPlace' must be called after 'SetAimOffsetVariable'

-----------------------
2. SetAimOffsetVariables(DeltaTime)
{
  if (CombatComponent == nullptr)  return;
  if (CombatComponent->EquippedWeapon == nullptr) return;

  if (GetVelocity().Size2D() > 0.f && GetCharacterMovement()->IsFalling()==true )  //stand still, not jumping
  {
	bUseControllerRotationYaw = true;

	BaseAimRotation_SinceStopMoving = GetBaseAimRotation();

	AO_Yaw = 0;
  }
  else //moving | jumping
  {
	bUseControllerRotationYaw = false; //need to change to true if using 'TurnInPlace'

	FRotator DeltaRotation = GetBaseAimRotation() - BaseAimRotation_SinceStopMoving;

	AO_Yaw = DeltaRotation.GetNormalized().Yaw;
  }

  AO_Pitch = GetBaseAimRotation().GetNormalized().Pitch;

  UE_LOG(LogTemp, Warning, TEXT("AO_Yaw: %f"), AO_Yaw);
}

-----------------------
3. SetTurnInPlaceVariables(DeltaTime)
{
  if (CombatComponent == nullptr)  return;
  if (CombatComponent->EquippedWeapon == nullptr) return;

  //change it back to true, from false by SetAimOffsetVariables(DeltaTime):
  bUseControllerRotationYaw = true;

  if (GetVelocity().Size2D() > 0 && GetCharacterMovement()->IsFalling()==true ) //moving | jumping
  {
	SetTurningInPlace(ETurningInPlace::RTIP_NoTurning);
  }
  else  //not moving, not jumping
  {
	if (TurningInPlace == ETurningInPlace::RTIP_NoTurning)
	{
	  AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero = AO_Yaw;
	}
	else //either TurnLeft or TurnRight
	{
	  AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero = FMath::FInterpTo(
		  AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero,
		  0.f,
		  DeltaTime,
		  InterpSpeed_Turning
	  );

	  AO_Yaw = AO_Yaw_SinceNoTurning_ToBeInterpolatedBackToZero;

	  if (abs(AO_Yaw) <= X_stop){
		 TurningInPlace = ETurningInPlace::RTIP_NoTurning;

		 BaseAimRotation_SinceStopMoving = GetBaseAimRotation();
	  }
	}

	if (AO_Yaw > 90.f){
	  SetTurningInPlace(ETurningInPlace::RTIP_TurnRight);
	}

	if (AO_Yaw < -90.f){
	  SetTurningInPlace(ETurningInPlace::RTIP_TurnLeft);
	}
  }
}
*/
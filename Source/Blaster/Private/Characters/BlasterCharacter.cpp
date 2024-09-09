// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "InputActionValue.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
// Called when the game starts or when spawned
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
}




// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedPlayerInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedPlayerInputComponent)
	{
 		EnhancedPlayerInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move );
		EnhancedPlayerInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
		EnhancedPlayerInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_Jump);
	}
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
	Super::Jump();
}
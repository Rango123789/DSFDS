// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Casing Mesh"));
	RootComponent = CasingMesh;

	//fix bullet blocking Camera:
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//for physics part: can also do it from BP
	CasingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	//this is C++ name of "Simulation Generates Hit Event":
	CasingMesh->SetNotifyRigidBodyCollision(true);

}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	//for Hit callback:
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnCasingMeshHit);

	//technically the X-axis of AmmoSocket is the direction of Velocity you want to apply
	//however1 Actor's transform = RootComp's Transform = CasingMesh's Transform
	//however2 you did spawn the shell with ACasing::Transform = AmmoSocket:Transform remember?
	//so now you need only pass in GetVectorForward() for the direction of the force you want
	CasingMesh->AddImpulse(GetActorForwardVector() * ImpulseMagnitude );
}

void ACasing::OnCasingMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());

	Destroy();  // .SetTimer( Destroy() , x) if you like to let it linger - not good for performance
}

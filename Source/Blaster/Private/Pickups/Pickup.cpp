// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Pickup.h"
#include <Components/SphereComponent.h>
#include <Kismet/GameplayStatics.h>
#include "NiagaraComponent.h"
#include <NiagaraFunctionLibrary.h>

#include "Pickups/PickupSpawnPoint.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; //like weapon, we surely need it to be replicated

	RootComponent = CreateDefaultSubobject<USceneComponent>("DefaultSceneComp");

	Sphere = CreateDefaultSubobject<USphereComponent>("SphereComp");
	Sphere->SetupAttachment(RootComponent);
	//it dont have physics with Query only or 'SimulatePhysics' unchecked so dont worry, it wont fall even we ignore all:
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	Sphere->SetGenerateOverlapEvents(true);
	//tips: instead of raise all childs up, simply do it here in parent LOL, this is Constructor, not Tick, so dont worry, it will only Add 'once' : 
	Sphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	//the mesh if picked, is just for cosmetic, no need collision
	//The reason why stephen make Mesh attach to Sphere instead of RootComp is that when we move Sphere it move the Mesh too
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMeshComp");
	PickupMesh->SetupAttachment(Sphere); //update from root
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	//optional, currently our weapon is 251=blue
	PickupMesh->SetCustomDepthStencilValue(250);
	PickupMesh->SetRenderCustomDepth(true);
	//the same logic here:
	PickupMesh->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));

	//move from PickupHealth:
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComp");
	NiagaraComponent->SetupAttachment(RootComponent);
}



// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (Sphere && HasAuthority())
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap); //&ThisClass::OnSphereOverlap
	}
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//funny stepehn rotate the mesh rather than the whole actor, each will retote locally:
	if (PickupMesh)
	{
		FRotator DeltaRotation(0.f, AnglePerSecond * DeltaTime  , 0.f);
		PickupMesh->AddWorldRotation(DeltaRotation);
	}
}

//this will only trigger in the server
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickup::Destroyed()
{
	Super::Destroyed();

	UGameplayStatics::PlaySoundAtLocation(this, PickSound, GetActorLocation());

	//move from APickup_Health, spawn extra Niagara asset when destroyed(): 
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, NiagaraSystem_SpawnedWhenDestroyed, GetActorLocation()); //autodestroy

	//this part is for working with APickupSpawnPoint (if any):
	//this pickup will die in a momenet, but as long as APickupSpawnPoint::StartTimer_SpawnPickup() dont use any DATA of this Pickup, then it should work fine!
	APickupSpawnPoint* PickupSpawnPoint = Cast<APickupSpawnPoint>(GetOwner());
	if (PickupSpawnPoint) PickupSpawnPoint->StartTimer_SpawnPickup();
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//add this line to allow it to be self-replicate after you add "HasAuthority()"
	bReplicates = true;
	
	//setup CollisionBox/ProjectileBox
	CollisionBox = CreateDefaultSubobject<UBoxComponent>("Weapon Box");
	RootComponent = CollisionBox; 
	
	//it is collision-capable comp, let's give it some settings:
	   //it is moved by logic, also by physics so we choose WorldDynamic:
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	   //we want it also have physics, we enable it right here rather than in BeginPlay() like for AWeapon::Sphere previously in this Multiplayer game.
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
	   //ignore all first, and block some later: 
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	   //so we want this projectile to be blocked by Static objects in world
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	   //we need it to block Visibility (not talk about Camera) so that we can do some trace on it I guess?
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	//Setup ProjectileMovementComp:
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Move Comp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
// Called when the game starts or when spawned

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		//TracerComponent =
			UGameplayStatics::SpawnEmitterAttached(
				Tracer,
				CollisionBox,
				FName(),
				GetActorLocation(),
				GetActorRotation(),
				EAttachLocation::KeepWorldPosition
		);
	}

	//Only the server projectile copy can generate hit event FIRST
	if (HasAuthority() && CollisionBox)
	{
		//i just add this, stephen didn't add this :)
		CollisionBox->SetNotifyRigidBodyCollision(true); //C++ name for Generate Hit Event from BP

		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnBoxHit);
	}
}

//currently only in-server projectile copy can trigger this
void AProjectile::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

//auto-call on all devices if the replicated actor is destroyed via AActor::Destroy() from the server
void AProjectile::Destroyed()
{
	Super::Destroyed();

	UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());     //Hit.ImpactPoint
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform());//Hit.ImpactPoint
}




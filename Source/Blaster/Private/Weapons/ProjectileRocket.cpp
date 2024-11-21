// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include <Characters/BlasterCharacter.h>
#include "NiagaraComponent.h"
#include <NiagaraFunctionLibrary.h>
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Weapons/ProjectileMovementComponent_Rock.h"

AProjectileRocket::AProjectileRocket()
{
	//this is only for cosmetic, no need collision, as its cousin ProjectileBullet dont even have a mesh!
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent_Rock = CreateDefaultSubobject<UProjectileMovementComponent_Rock>("Projectile Move Comp");
	ProjectileMovementComponent_Rock->bRotationFollowsVelocity = true;
	ProjectileMovementComponent_Rock->InitialSpeed = 300;
	ProjectileMovementComponent_Rock->MaxSpeed = 700;

	//Stephen recommend, yes we need this as we did seriously already the default implementation, that we stop HandleImpact() with empty code, even if the built-in version is self-replicated
	//this is needed here (not not in bullet)
	//UPDATE] to be exact the PURE version also need it in case you  'return from first hit in your code'
	ProjectileMovementComponent_Rock->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();


	SpawnSmokeTrailSystem();

	//if (NiagaraSystem_SmokeTrail)
	//{
	//	NiagaraComponent_SmokeTrail = UNiagaraFunctionLibrary::SpawnSystemAttached(
	//		NiagaraSystem_SmokeTrail,
	//		RootComponent,
	//		FName(),
	//		GetActorLocation(),
	//		FRotator(),
	//		EAttachLocation::KeepWorldPosition,
	//		true //bAutoDestroy
	//	);
	//}
	
	
	//Set bAutoDestroy = true is just extra of safety when we want it stop before Actor::Destroy() is called, set it to false like Stephen doesn't actually change anything :D
	AudioComponent =
		UGameplayStatics::SpawnSoundAttached(
			RocketMovingSound, 
			RootComponent, 
			FName(), 
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			true, //bStopWhenAttachedToDestroyed
			1,1,0, //starting time 0 <=> play at time 0 rather than skip some!
			(USoundAttenuation*)nullptr, //you dont need Att when the sound asset itself already have it
			(USoundConcurrency*)nullptr,
			true //bAutoDestroyed - not work for looing sound or will die when the hosting Actor::Comp::Var die?? - no it auto destroy exactly like N Asset
	);

	//OPTION1: to solve 'non-replicated DoActions' after mess it up LOL
	//the HasAuthority has been turn on from parent, now in child we only need to turn on the rest lol
	if (!HasAuthority() && CollisionBox)
	{
		//i just add this, stephen didn't add this :)
		CollisionBox->SetNotifyRigidBodyCollision(true); //C++ name for Generate Hit Event from BP

		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnBoxHit);
	}
}


//we just set collision backup to work in clients as well, this not just run on the server any more:
void AProjectileRocket::OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetInstigator()) return;

//GROUP1: must be done in the server first as a first requirement,previously replicated or you have to make it so, say call RPC inside this same group, and dont disturb this same pattern:
	//it must be shot from someone, it must have an instigator LOL
	if (HasAuthority())
	{
		AController* InstagatorController = GetInstigator()->GetController();
		if (InstagatorController == nullptr) return;

		////We dont do this for Rocket , we need to apply Radial Damage instead:
		//UGameplayStatics::ApplyDamage(
		//	Damaged_BlasterCharacter,
		//	Damage,
		//	GetInstigator()->GetController(), //Instigator of this Projectile, of APawn type, was set to the Character holding the Weapon that this Projectile spawn from
		//	this,
		//	UDamageType::StaticClass()
		//);

		//we need to apply Radial Damage instead:
		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this, //WorldContextObject
			Damage, //BaseDamage 
			10.f,   //MinimumDanage
			GetActorLocation(), //Origin of the radial range
			200.f, //DamgeInnerRadius, this is radius NOT Damage LOL
			500.f,  //DamageOuterRadius
			1.f,   // 1^[X] , here X = 1.f
			UDamageType::StaticClass(),
			TArray<AActor*>{},     //IgnoreActors
			this,                  //DamageCauser
			InstagatorController,  //InstigatedByController
			ECollisionChannel::ECC_Visibility
		);
	}

//GROUP2: non-replicated (or self-replicated) that can be done in whatever way, dont add any condition, 'OPTIONALLY' if any of them is 'self-replicated' then you can move them up to GROUP1 (avoiding potential side effect by circle replication)
	//move off-timing from Destroyed() here:
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());  //NOT replicated
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform()); //NOT replicated
	//Other things:
	if (ProjectileMesh) ProjectileMesh->SetVisibility(false); //self-replicated?? - NO
	//because the box is linger there for Delaytime, and its collision is till funcking working, hence turn it of for good lol, i miss this one!
	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Deactivate N asset rather than Destroy its Actor+HostingComp, and set a timer to Destroy its Actor+HostingComp, so that Particle can linger a bit:
	if(NiagaraComponent_SmokeTrail) NiagaraComponent_SmokeTrail->Deactivate(); //self-replicated?? - NO
	//the AudioComponent->IsPlaying() is redudant:
	if (AudioComponent && AudioComponent->IsPlaying()) AudioComponent->Stop();

	StartDestroyTimer();
	//GetWorldTimerManager().SetTimer(TimerHandle_Destroy, this, &ThisClass::TimerCallback_Destroy, DelayTime_Destroy, false); //self-replicated - can optionally move it up to GROUP1
	
	//We must not call supper that has 'Destroy()' code, now we do it in Timer Delay=3s instead
	//Super::OnBoxHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

//void AProjectileRocket::TimerCallback_Destroy()
//{
//	Destroy();
//}

//we override to 'UNDO' to DoAction in Super::Destroyed() that is pasted into BoxHit already.
void AProjectileRocket::Destroyed()
{
	//do nothing , dont call Super:: even
}


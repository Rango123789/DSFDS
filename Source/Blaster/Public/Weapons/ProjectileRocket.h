// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void OnBoxHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;


	//UPROPERTY(VisibleAnywhere)
	//class UStaticMeshComponent* ProjectileMesh;

	//UPROPERTY(EditAnywhere)
	//class UNiagaraSystem* NiagaraSystem_SmokeTrail; //SmokeEffect for N asset Vs SmokeParticle for P asset
	//UPROPERTY()
	//class UNiagaraComponent* NiagaraComponent_SmokeTrail;

	////no need to create callback, the callback is 'void Destroy()' itself satisied signature requirement? No it require UFUNCTION(), where AActor::Destroy() is not a UFunction()
	////you see it require TFunction ~> I guess it is 'function marked with UFUNCTION'
	//FTimerHandle TimerHandle_Destroy;
	//UPROPERTY(EditAnywhere)
	//float DelayTime_Destroy = 3.f;
	//UFUNCTION()
	//void TimerCallback_Destroy();

	UPROPERTY(EditAnywhere)
	USoundBase* RocketMovingSound;

	UPROPERTY()
	UAudioComponent* AudioComponent;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent_Rock* ProjectileMovementComponent_Rock;

};

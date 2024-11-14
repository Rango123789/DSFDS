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
protected:
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* RocketMesh1;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* NS_SmokeTracer; //SmokeEffect for N asset Vs SmokeParticle for P asset

};

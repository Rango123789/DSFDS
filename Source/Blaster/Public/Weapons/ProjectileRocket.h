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
protected:
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* RocketMesh1;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* NS_SmokeTracer; //SmokeEffect for N asset Vs SmokeParticle for P asset
	UPROPERTY()
	class UNiagaraComponent* NiagaraComponent_SmokeTracer;
	//no need to create callback, the callback is 'void Destroy()' itself satisied signature requirement? No it require UFUNCTION(), where AActor::Destroy() is not a UFunction()
	//you see it require TFunction ~> I guess it is 'function marked with UFUNCTION'
	FTimerHandle TimerHandle_Destroy;
	UPROPERTY(EditAnywhere)
	float DelayTime_Destroy = 3.f;
	UFUNCTION()
	void TimerCallback_Destroy();

	UPROPERTY(EditAnywhere)
	USoundBase* RocketMovingSound;

	UPROPERTY()
	UAudioComponent* AudioComponent;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent_Rock* ProjectileMovementComponent_Rock;

};

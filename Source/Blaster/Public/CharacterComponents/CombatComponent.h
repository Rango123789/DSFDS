// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Equip(class AWeapon* InWeapon);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_EquippedWeapon();
protected:
	virtual void BeginPlay() override;

private:
//***function***
	void SetIsAiming(bool InIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool InIsAiming);

	void Input_Fire(bool InIsFiring);

	UFUNCTION(Server, Reliable)
	void ServerInput_Fire(bool InIsFiring, const FVector_NetQuantize& Target);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastInput_Fire(bool InIsFiring, const FVector_NetQuantize& Target);

	//stephen name it 'TraceUnderCrosshairs' . In last course we name BoxHit ->better BoxHitResult
	void DoLineTrace_UnderCrosshairs(FHitResult& LineHitResult);

//***data member***
	//this no need to be replicated, it is set for all version back in Char::PostInitializeComponents
	class ABlasterCharacter* Character; //to let this comp aware of its hosting object

	//the Equipped Pose relying on this to know whether Char has a weapon or not, so that to choose "which group of anims: equipped or not"
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon) //just upgrade it to 'Using' for fixing a client can't change bOrient on itself :D :D
	class AWeapon* EquippedWeapon;      //and more
	
	UPROPERTY(Replicated)
	bool bIsAiming{};

	UPROPERTY(Replicated)
	bool bIsFiring{};

	UPROPERTY(EditAnywhere)
	float MaxWalkSpeed_Backup; //backup for initial MaxWalkSpeed, set its value in constructor!
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed; //to change MaxWalkSpeed = AimWalkSpeed when we aim

	//UPROPERTY(Replicated)  didn't work
	FVector HitTarget;

	UPROPERTY(EditAnywhere)
	float SphereRadius = 50.f;
	UPROPERTY(EditAnywhere)
	bool bDrawConsistentLine = false;
public:	
	friend class ABlasterCharacter;     //since already forward-declare, so 'class' here is optional!


	
};

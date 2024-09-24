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
protected:
	virtual void BeginPlay() override;

private:
	//this no need to be replicated, it is set for all version back in Char::PostInitializeComponents
	class ABlasterCharacter* Character; //to let this comp aware of its hosting object

	//the Equipped Pose relying on this to know whether Char has a weapon or not, so that to choose "which group of anims: equipped or not"
	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;      //and more
	
	UPROPERTY(Replicated)
	bool bIsAiming{};


public:	
	friend class ABlasterCharacter;     //since already forward-declare, so 'class' here is optional!

};

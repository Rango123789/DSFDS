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

	void EquipWeapon(class AWeapon* InWeapon);
protected:
	virtual void BeginPlay() override;

private:
	class ABlasterCharacter* Character; //to let this comp aware of its hosting object
	class AWeapon* EquippedWeapon;  //and more

public:	
	friend class ABlasterCharacter; //since already forward-declare, so 'class' here is optional!

};

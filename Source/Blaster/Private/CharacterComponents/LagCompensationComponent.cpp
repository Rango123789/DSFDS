// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/LagCompensationComponent.h"
#include "Characters/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Components/BoxComponent.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);
	ShowFramePackage(FramePackage, FColor::Orange);

}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& FramePackage)
{
	//Stephen now confirm that owner of UActorComponent is the hosting actor having it as component (created by CreateDefaultSubobject)
	//Anyway since we did set it in Char::PostInitializeComponents, and Combat didn't even need to re-check this so I think this is REDUDANT for me:
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character == nullptr) return;

//FramPackage::  Time + TMap<FName, FBoxInfor> ; FBoxInfo = Localtion + Rotation + BoxExtent 
	//Sub member1: Time
		//we only plan to do it from server, so simply do 'GetWorld()->GetTimeSeconds()' will be enough, rather than call PC::GetServerTime() in worst case for any device!
	FramePackage.Time = GetWorld()->GetTimeSeconds();
	//sub member2: TMap<FName, FBoxInfor> , so we need to add elements for the map many times

	FBoxInfo BoxInfo;
		//BoxComponentMap is setup from Char, and all DATA is ready.
	for (auto Pair : Character->BoxComponentMap)
	{
		//this is SUB struct, hence we create a temp to store it before assign it to GLOBAL struct
		BoxInfo.Location = Pair.Value->GetComponentLocation();
		BoxInfo.Rotation = Pair.Value->GetComponentRotation();
		BoxInfo.BoxExtent = Pair.Value->GetScaledBoxExtent();
		//the key is the key of BoxComponentMap from Char itself, hell yeah!
		FramePackage.BoxInfoMap.Add(Pair.Key, BoxInfo);
	}
}

//we only need FramePackage.BoxInfoMap to draw them, not need .Time:
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& FramePackage, const FColor& Color)
{
	for (const auto& Pair : FramePackage.BoxInfoMap)
	{
		//For now bPersisten = true, but next lesson Do not draw persisten line that cost performance, Draw it for 4s that is the time of Frame History
		DrawDebugBox(GetWorld(), Pair.Value.Location, Pair.Value.BoxExtent, FQuat(Pair.Value.Rotation), Color, true, -1.f, 0 , 1.f); //false, 4.f
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


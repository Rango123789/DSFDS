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

	//FFramePackage FramePackage;
	//SaveFramePackage(FramePackage);
	//ShowFramePackage(FramePackage, FColor::Orange);











}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& FramePackage)
{
	//Stephen now confirm that owner of UActorComponent is the hosting actor having it as component (created by CreateDefaultSubobject)
	//Anyway since we did set it in Char::PostInitializeComponents, and Combat didn't even need to re-check this so I think this is REDUDANT for me:
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character == nullptr || GetWorld() == nullptr) return;

//FramPackage::  Time + TMap<FName, FBoxInfor> ; FBoxInfo = Localtion + Rotation + BoxExtent 
	//Sub member1: Time
		//we only plan to do it from server, so simply do 'GetWorld()->GetTimeSeconds()' will be enough, rather than call PC::GetServerTime() in worst case for any device!
	FramePackage.Time = GetWorld()->GetTimeSeconds();
	//sub member2: TMap<FName, FBoxInfor> , so we need to add elements for the map many times

	FBoxInfo BoxInfo;
		//BoxComponentMap is setup from Char, and all DATA is ready.
	for (const auto& Pair : Character->BoxComponentMap)
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
		//DrawDebugBox(GetWorld(), Pair.Value.Location, Pair.Value.BoxExtent, FQuat(Pair.Value.Rotation), Color, true, -1.f, 0 , 1.f); 
		DrawDebugBox(GetWorld(), Pair.Value.Location, Pair.Value.BoxExtent, FQuat(Pair.Value.Rotation), Color, false, 4.f, 0, 1.f); //false, 4.f
	}
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);

//UPDATE: current convention of grow direction: [tail -> head]  (doesn't matter C++ usual order, but match the timelife history and so many other reasons, so I choose to follow stephen)
	//either one of if,else if, else could be executed per frame; no matter which one is added, we always add
	//you can in fact merge the first two into into .AddHead() - [tail->head] = STEPHEN
		//add first tail (0)  when there is no element (or just one element = head = tail)
	if (FramePackageList.Num() == 0)
	{
		FramePackageList.AddTail(FramePackage);
	}
	//add first head (1) when there is already one element, meaning we choose this convention: tail (old)-> head (new) ; then continue to remove tail(old) and add head(new) when it goes beyond MaxTimeRecord
	else if (FramePackageList.Num() == 1)
	{
		FramePackageList.AddHead(FramePackage);
	}
	else
	{
		//our current convention is tail LATER, so Time of HEAD must be bigger!
		//after we know that we surely have 2+ elements when enter this, stephen dont even check "GetHead() or GetTail() nullptr or not" - not sure this is safe? 
		float HistoryLength =
			FramePackageList.GetHead()->GetValue().Time - FramePackageList.GetTail()->GetValue().Time;
		//we remove head (or a number of them) until HistoryLength <= MaxRecordTime
		while (HistoryLength > MaxRecordTime)
		{
			FramePackageList.RemoveNode(FramePackageList.GetTail());
			//after we remove one head (old), we recount the time to see if we should need to remove more:
			HistoryLength =
				FramePackageList.GetHead()->GetValue().Time - FramePackageList.GetTail()->GetValue().Time;
		}

		//After that we always add 'the LASTEST FramePackage' of the current frame (could make HistoryLength go beyond MaxRecordTime again, but it doesn't matter, it is just one frame, and it will be removed the tail to compensate in next frame in the while loop above anyway:
		FramePackageList.AddHead(FramePackage);
	}

	//Draw FramePackage per frame
		//this work is absolutely indendendent from how we store above lol, we simply draw 'FramePackage' after the first 2 lines of code! so it doesn't tell whether your storing work is correct
		// So you may need to 'Draw per frame' and interator over the list back to draw all elements of it to see if you're storing it correctly, but anyway we will know in next lesson, so I will folow stephen anway
		// This is expensive, so do not use bPersistent true, let the LifeTime = MaxRecordTime is alsolute perfect!

	ShowFramePackage(FramePackage, FColor::Orange);

}


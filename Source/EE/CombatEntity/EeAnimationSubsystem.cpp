// Fill out your copyright notice in the Description page of Project Settings.


#include "EeAnimationSubsystem.h"

#include "MassEntitySubsystem.h"

void UEeAnimationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Initialize dependent subsystems before calling super
	Collection.InitializeDependency(UMassEntitySubsystem::StaticClass());
	
	Super::Initialize(Collection);

	// In here you can hook to delegates!
	// ie: OnFireHandle = FExample::OnFireDelegate.AddUObject(this, &UMyWorldSubsystem::OnFire);
}

void UEeAnimationSubsystem::Deinitialize()
{
	// In here you can unhook from delegates
	// ie: FExample::OnFireDelegate.Remove(OnFireHandle);
	AnimationTimingMap.Empty();
	Super::Deinitialize();
}

void UEeAnimationSubsystem::AddUnitTypeAnimTimings(FString Name, FString AnimType, float InStart, float InEnd, float InPlayRate, float InFrameRate)
{
	if (!AnimationTimingMap.Contains(Name))
	{
		TMap<FString, FAnimationTiming> InsideTimingMap;
		InsideTimingMap.Add(AnimType,FAnimationTiming(InStart,InEnd,InPlayRate,InFrameRate));
		AnimationTimingMap.Add(Name,InsideTimingMap);
		return;
	}
	if (!AnimationTimingMap[Name].Contains(AnimType))
	{
		AnimationTimingMap[Name].Add(AnimType,FAnimationTiming(InStart,InEnd,InPlayRate,InFrameRate));
		return;
	}
	AnimationTimingMap[Name][AnimType] = FAnimationTiming(InStart,InEnd,InPlayRate,InFrameRate);
}

FAnimationTiming UEeAnimationSubsystem::GetAnimation(FString Name, FString AnimType) const
{
	if (!AnimationTimingMap.Contains(Name))
	{
		return FAnimationTiming(0,30,1);
	}
	if (!AnimationTimingMap[Name].Contains(AnimType))
	{
		return AnimationTimingMap[Name]["Idle"];
	}
	return AnimationTimingMap[Name][AnimType];
}

TArray<FString> UEeAnimationSubsystem::GetMeshNames() const
{
	TArray<FString> MeshNames;
	for (auto MeshName : AnimationTimingMap)
	{
		MeshNames.Add(MeshName.Key);
	}
	return MeshNames;
}
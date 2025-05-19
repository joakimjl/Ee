// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "CoreMinimal.h"
#include "MassSubsystemBase.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/IntVector.h"

#include "EeAnimationSubsystem.generated.h"

/**
 * Struct that contains Animation Timing values for the EeAnimationSubsystem
 */
USTRUCT(BlueprintType)
struct FAnimationTiming
{
	GENERATED_BODY()

	UPROPERTY()
	float Start;
	UPROPERTY()
	float End;
	UPROPERTY()
	float PlayRate;
	UPROPERTY()
	float FrameRate;

	FAnimationTiming(float InStart = 0.f, float InEnd = 30.f, float InPlayRate = 1.f, float InFrameRate = 30.f)
		: Start(InStart), End(InEnd), PlayRate(InPlayRate), FrameRate(InFrameRate)
	{}
};

/** Subsystem handling Vertex Animation playing */
UCLASS()
class EE_API UEeAnimationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void AddUnitTypeAnimTimings(FString Name, FString AnimType, float InStart, float InEnd, float InPlayRate, float InFrameRate);
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetMeshNames() const;
	UFUNCTION(BlueprintCallable)
	FAnimationTiming GetAnimation(FString Name, FString AnimType) const;

protected:
	// UWorldSubsystem begin interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UWorldSubsystem end interface
	
private:
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(AccessDetector);
	TMap<FString, TMap<FString, FAnimationTiming>> AnimationTimingMap;
};

template<>
struct TMassExternalSubsystemTraits<UEeAnimationSubsystem> final
{
	enum
	{
		ThreadSafeRead = true,
		ThreadSafeWrite = false,
		GameThreadOnly = false
	};
};
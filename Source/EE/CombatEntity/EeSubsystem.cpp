// Fill out your copyright notice in the Description page of Project Settings.


#include "EeSubsystem.h"

#include "CombatFragments.h"
#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "ToolContextInterfaces.h"


void UEeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Initialize dependent subsystems before calling super
	Collection.InitializeDependency(UMassEntitySubsystem::StaticClass());
	
	Super::Initialize(Collection);
	
	EntityHandleGrid = TMap<int32, FEntityHandleGridCellY>();
	EeEntityManager = &GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	// In here you can hook to delegates!
	// ie: OnFireHandle = FExample::OnFireDelegate.AddUObject(this, &UMyWorldSubsystem::OnFire);
}

void UEeSubsystem::Deinitialize()
{
	// In here you can unhook from delegates
	// ie: FExample::OnFireDelegate.Remove(OnFireHandle);
	EntityHandleGrid.Empty();
	Super::Deinitialize();
}

FIntVector2 UEeSubsystem::VectorToGrid(FVector Vector)
{
	FIntVector2 Out;
	Out.X = FMath::FloorToInt(Vector.X/200);
	Out.Y = FMath::FloorToInt(Vector.Y/200);
	return Out;
}

FVector UEeSubsystem::MoveSelfInGrid(FMassEntityHandle Handle, FVector PreviousLocation)
{
	FIntVector2 GridVec = VectorToGrid(PreviousLocation);
	if (EntityHandleGrid.Contains(GridVec.X))
	{
		if (EntityHandleGrid[GridVec.X].InnerMap.Contains(GridVec.Y))
		{
			if (EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Contains(Handle)) EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Remove(Handle);
		}
	}
	FVector NewLocation = AddSelfToGrid(Handle);
	return NewLocation;
}

FVector UEeSubsystem::AddSelfToGrid(FMassEntityHandle Handle)
{
	const FTransformFragment* TransformFrag = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(Handle);
	if (TransformFrag == nullptr) return FVector::ZeroVector;
	FVector Location = TransformFrag->GetTransform().GetLocation();
	FIntVector2 GridVec = VectorToGrid(Location);
	if (EntityHandleGrid.Contains(GridVec.X))
	{
		if (EntityHandleGrid[GridVec.X].InnerMap.Contains(GridVec.Y))
		{
			if (EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Contains(Handle)) EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Add(Handle);
		}
	}
	return Location;
}


bool UEeSubsystem::SpawnProjectile(FMassEntityHandle Handle)
{
	TArray<FMassEntityHandle> EntitiesProjectiles;
	const UScriptStruct* FragmentTypes[] = { FTransformFragment::StaticStruct(), FProjectileParams::StaticStruct(), FProjectileFragment::StaticStruct() };
	const FMassArchetypeHandle ArchetypeHandle = EeEntityManager->CreateArchetype(MakeArrayView(FragmentTypes, 3));
	EeEntityManager->BatchCreateEntities(ArchetypeHandle, 1, EntitiesProjectiles);

	return true;
}

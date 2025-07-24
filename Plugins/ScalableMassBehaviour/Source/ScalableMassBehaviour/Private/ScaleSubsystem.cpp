// Fill out your copyright notice in the Description page of Project Settings.


#include "ScaleSubsystem.h"
#include "MassEntityBuilder.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "ScaleFragments.h"
#include "ScalePhysicsManager.h"

void UScaleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UMassEntitySubsystem::StaticClass());
	Super::Initialize(Collection);
	
	UMassEntitySubsystem* MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	EntityManagerPtr = &MassEntitySubsystem->GetMutableEntityManager();
	UE::Mass::FEntityBuilder Builder(*EntityManagerPtr);
	EntityBuilder = &Builder;
	
	Grid = NewObject<UGrid>();

	RegisteredResources = TMap<EProcessable, FProcessableArr>();

	FProcessableReqArr ProcessReqArray = FProcessableReqArr();
}

void UScaleSubsystem::Deinitialize()
{
	EntityBuilder = nullptr;
	EntityManagerPtr = nullptr;
	
	Super::Deinitialize();
}

void UScaleSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*
	TimeSinceRemoval+=DeltaTime;
	if (TimeSinceRemoval >= 0.01f)
	{
		TimeSinceRemoval = 0;
		DestroyStalledEntity();
	} */
}

TStatId UScaleSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScaleSubsystem, STATGROUP_Tickables);
}

bool UScaleSubsystem::RegisterResource(FVector Location, EProcessable Type, UMassAgentComponent* Component)
{
	if (!RegisteredResources.Contains(Type)) RegisteredResources.Add(Type, FProcessableArr());
	RegisteredResources[Type].Locations.Add(Location);
	//FMassEntityHandle New = EntityManagerPtr->ReserveEntity();
	Component->Enable();
	//Component->ClearEntityHandle();
	//Component->SetEntityHandle(New);
	RegisteredResources[Type].Handles.Add(Component->GetEntityHandle());
	return true;
}

TArray<FVector> UScaleSubsystem::GetResources(EProcessable Type)
{
	if (!RegisteredResources.Contains(Type)) return TArray<FVector>();
	return RegisteredResources[Type].Locations;
}

FMassEntityHandle UScaleSubsystem::GetClosestResource(EProcessable Type, FVector Location)
{
	if (!RegisteredResources.Contains(Type)) return FMassEntityHandle();
	float MinDist = MAX_FLT;
	if (RegisteredResources[Type].Locations.Num() <= 0) return FMassEntityHandle();
	FMassEntityHandle ClosestHandle = RegisteredResources[Type].Handles[0];
	for (int i = 0; i < RegisteredResources[Type].Locations.Num(); ++i)
	{
		FVector ResourceLocation = RegisteredResources[Type].Locations[i];
		if ( (Location-ResourceLocation).Size() >= MinDist ) continue;
		//UE_LOG(LogTemp, Display, TEXT("Closest Handle is... %i"), static_cast<int32>(RegisteredResources[Type].Handles[i].AsNumber()));
		MinDist = (Location-ResourceLocation).Size();
		ClosestHandle = RegisteredResources[Type].Handles[i];
	}
	return ClosestHandle;
}

bool UScaleSubsystem::AddToEntity(FMassEntityHandle EntityHandle, EProcessable Type, int32 Amount) const
{
	FResourceFragment* ResourceFragment = EntityManagerPtr->GetFragmentDataPtr<FResourceFragment>(EntityHandle);
	if (!ResourceFragment) return false;
	if (!ResourceFragment->Carrying.Contains(Type)) ResourceFragment->Carrying.Add(Type, 0); 
	EntityManagerPtr->GetFragmentDataPtr<FResourceFragment>(EntityHandle)->Carrying[Type] += Amount;

	//UE_LOG(LogTemp, Display, TEXT("Entity now has %i"), ResourceFragment->Carrying[Type]);
	
	return true;
}

bool UScaleSubsystem::RemoveFromEntity(FMassEntityHandle EntityHandle, EProcessable Type, int32 Amount) const
{
	FResourceFragment* ResourceFragment = EntityManagerPtr->GetFragmentDataPtr<FResourceFragment>(EntityHandle);
	if (!ResourceFragment) return false;
	if (!ResourceFragment->Carrying.Contains(Type)) return false;
	if (ResourceFragment->Carrying[Type] < Amount) return false;
	ResourceFragment->Carrying[Type] -= Amount;

	return true;
}

int32 UScaleSubsystem::GetEntityResources(EProcessable Type, FVector Location)
{
	FMassEntityHandle Handle = GetClosestResource(Type, Location);
	if (!EntityManagerPtr->IsEntityValid(Handle)) return 1;
	FResourceFragment& ResourceFragment = EntityManagerPtr->GetFragmentDataChecked<FResourceFragment>(Handle);
	//FResourceFragment* ResourceFragment = EntityManagerPtr->GetFragmentDataPtr<FResourceFragment>(Handle);
	//if (!ResourceFragment) return 0;
	int32 Total = 1;
	if (ResourceFragment.Carrying.Num() == 0) return 1;
	for (auto& [Key, Value] : ResourceFragment.Carrying)
	{
		Total += Value;
	}
	return Total;
} 

FVector UScaleSubsystem::GetEntityLocation(FMassEntityHandle Handle)
{
	return EntityManagerPtr->GetFragmentDataPtr<FTransformFragment>(Handle)->GetTransform().GetLocation();
}

FVector UScaleSubsystem::GetEntityDataLocation(FSmbEntityData EntityData)
{
	FMassEntityHandle Handle = FMassEntityHandle(EntityData.Index, EntityData.SerialNumber);
	if (!EntityManagerPtr->IsEntityValid(Handle)) return FVector();
	return EntityManagerPtr->GetFragmentDataPtr<FTransformFragment>(Handle)->GetTransform().GetLocation();
}

TArray<FMassEntityHandle> UScaleSubsystem::GetNumberClosestEntities(FVector Location, float Radius, int32 Amount)
{
	FVector2D Cell = VectorToCell(Location);
	int32 RadiusInCell = Radius/CellSize;
	int32 InsideRadius = 1+RadiusInCell;
	TArray<FMassEntityHandle> UnitArr = Grid->GetAround(Cell.X,Cell.Y,InsideRadius);
	TArray<FMassEntityHandle> ClosestArr = TArray<FMassEntityHandle>();

	TArray<TPair<float, FMassEntityHandle>> DistanceArray;
	for (auto Unit : UnitArr)
	{
		
		if (!EntityManagerPtr->IsEntityValid(Unit)) continue;
		FTransformFragment* TransformFragment = EntityManagerPtr->GetFragmentDataPtr<FTransformFragment>(Unit);
		if (!TransformFragment) continue;
        
		float Distance = (Location - TransformFragment->GetTransform().GetLocation()).Size();
		if (Distance <= Radius)
		{
			DistanceArray.Add(TPair<float, FMassEntityHandle>(Distance, Unit));
		}
	}
    
	DistanceArray.Sort([](const TPair<float, FMassEntityHandle>& A, const TPair<float, FMassEntityHandle>& B) {
		return A.Key < B.Key;
	});
    
	int32 ResultCount = FMath::Min(Amount, DistanceArray.Num());
	for (int32 i = 0; i < ResultCount; i++)
	{
		ClosestArr.Add(DistanceArray[i].Value);
	}
    
	return ClosestArr;
}

bool UScaleSubsystem::RegisterPhysicsManager(AScalePhysicsManager* InScalePhysicsManager, FString MeshName)
{
	FPhysicsManagerStruct PhysicsManager = FPhysicsManagerStruct();

	PhysicsManager.PhysicsManagerPtr = InScalePhysicsManager;
	PhysicsManager.MeshName = MeshName;

	//UE_LOG(LogTemp, Warning, TEXT("Registering physics manager %s"), *MeshName);
	
	PhysicsManagers.Add(PhysicsManager);
	return true;
}

void UScaleSubsystem::NewDeath(FString MeshName, int32 TotalDeaths, TArray<FVector> Locations)
{
	TArray<FVector> DeathLocations = TArray<FVector>();
	DeathLocations.Add((Locations[Locations.Num()-1]+FVector::UpVector*10.f));
	for (auto PhysicsManagerStruct : PhysicsManagers)
	{
		if (PhysicsManagerStruct.MeshName == MeshName){
			PhysicsManagerStruct.PhysicsManagerPtr->AddPhysicsParticles(Locations, TotalDeaths);
			//UE_LOG(LogTemp, Warning, TEXT("Added death particles %i locations and %i deaths"),Locations.Num(), TotalDeaths);
			return;
		}
	}
}


FSmbEntityData UScaleSubsystem::GetClosestEnemy(FVector Location, int32 TeamId, float Radius)
{
	FVector2D Cell = VectorToCell(Location);
	int32 RadiusInCell = Radius/CellSize;
	int32 InsideRadius = 1+RadiusInCell;
	TArray<FMassEntityHandle> UnitArr = Grid->GetAround(Cell.X,Cell.Y,InsideRadius);

	float MinDist = MAX_FLT;
	FSmbEntityData ClosestData = FSmbEntityData();

	//UE_LOG(LogTemp, Warning, TEXT("Checking %i units"), UnitArr.Num());

	for (auto Unit : UnitArr)
	{
		if (!EntityManagerPtr->IsEntityValid(Unit)) continue;
		FTransformFragment* TransformFragment = EntityManagerPtr->GetFragmentDataPtr<FTransformFragment>(Unit);
		if (!TransformFragment) continue;
		FTeamFragment* TeamFragment = EntityManagerPtr->GetFragmentDataPtr<FTeamFragment>(Unit);
		if (!TeamFragment) continue;
		if (TeamFragment->TeamID == TeamId) continue;
		FDefenceFragment* DefenceFrag = EntityManagerPtr->GetFragmentDataPtr<FDefenceFragment>(Unit);
		if (!DefenceFrag) continue;
		if (DefenceFrag->HP <= 0) continue;
		float Dist = (Location-TransformFragment->GetTransform().GetLocation()).Size();
		if (Dist >= MinDist) continue;
		MinDist = Dist;
		ClosestData.SerialNumber = Unit.SerialNumber;
		ClosestData.Index = Unit.Index;
	}
	return ClosestData;
}

bool UScaleSubsystem::DealDamageToEnemy(FSmbEntityData TargetData, float DamageAmount, EDamageType DamageType)
{
	if (TargetData.SerialNumber == -1 && TargetData.Index == -1) return false;
	//UE_LOG(LogTemp, Warning, TEXT("Checking handle and is %i %i"), TargetData.Index, TargetData.SerialNumber)
	FMassEntityHandle EnemyHandle = FMassEntityHandle(TargetData.Index, TargetData.SerialNumber);
	if (!EntityManagerPtr->IsEntityValid(EnemyHandle)) return false;
	//UE_LOG(LogTemp, Warning, TEXT("Entity Valid and was %i %i"), TargetData.Index, TargetData.SerialNumber)
	FDefenceFragment* DefenceFragmentPtr = EntityManagerPtr->GetFragmentDataPtr<FDefenceFragment>(EnemyHandle);
	if (!DefenceFragmentPtr) return false;
	if (DamageType == EDamageType::Blunt && DefenceFragmentPtr->UnitArmour == EArmourType::HeavyArmour)
		DamageAmount *= 2;
	if (DamageType == EDamageType::Slashing && DefenceFragmentPtr->UnitArmour == EArmourType::LightArmour)
		DamageAmount *= 2;
	if (DamageType == EDamageType::Piercing && DefenceFragmentPtr->UnitArmour == EArmourType::MediumArmour)
		DamageAmount *= 2;
	DefenceFragmentPtr->HP -= DamageAmount;
	//UE_LOG(LogTemp, Warning, TEXT("Dealt damage"))
	if (DefenceFragmentPtr->HP <= 0)
	{
		DefenceFragmentPtr->HP = 0;
	}
	return true;
}

FVector UScaleSubsystem::RegisterToGrid(FVector NewLocation, FMassEntityHandle Handle, FVector OldLocation)
{
	if (!EntityManagerPtr->IsEntityValid(Handle)) return FVector::DownVector;
	FVector2D NewCell = VectorToCell(NewLocation);
	FVector2D OldCell = VectorToCell(OldLocation);
	Grid->RemoveAt(OldCell.X,OldCell.Y,Handle);
	Grid->AddToGrid(NewCell.X,NewCell.Y,Handle);
	return NewLocation;
}

bool UScaleSubsystem::DealDamageAoe(FVector InLocation, float Radius, float DamageAmount, EDamageType DamageType, int32 OwnTeam)
{
	FVector2D CellLocation = VectorToCell(InLocation);
	TArray<FMassEntityHandle> EnemyArray = Grid->GetAround(CellLocation.X,CellLocation.Y,Radius);

	for (auto EnemyHandle : EnemyArray)
	{
		if (!EntityManagerPtr->IsEntityValid(EnemyHandle)) continue;
		FTransformFragment* TransformFragment = EntityManagerPtr->GetFragmentDataPtr<FTransformFragment>(EnemyHandle);
		if (!TransformFragment) continue;
		FVector EnemyLocation = TransformFragment->GetMutableTransform().GetLocation();
		if ((EnemyLocation-InLocation).Size()>Radius) continue;
		FDefenceFragment* DefenceFragment = EntityManagerPtr->GetFragmentDataPtr<FDefenceFragment>(EnemyHandle);
		if (!DefenceFragment) continue;
		FTeamFragment* TeamFragment = EntityManagerPtr->GetFragmentDataPtr<FTeamFragment>(EnemyHandle);
		if (!TeamFragment) continue;
		if (TeamFragment->TeamID == OwnTeam) continue;
		if (DamageType == EDamageType::Blunt && DefenceFragment->UnitArmour == EArmourType::HeavyArmour)
			DamageAmount *= 2;
		if (DamageType == EDamageType::Slashing && DefenceFragment->UnitArmour == EArmourType::LightArmour)
			DamageAmount *= 2;
		if (DamageType == EDamageType::Piercing && DefenceFragment->UnitArmour == EArmourType::MediumArmour)
			DamageAmount *= 2;
		DefenceFragment->HP -= DamageAmount;
		if (DefenceFragment->HP <= 0) DefenceFragment->HP = 0;
	}
	
	return true;
}

void UScaleSubsystem::DestroyEntity(FMassEntityHandle Handle)
{
	if (!EntityManagerPtr->IsEntityValid(Handle)) return;
	FLocationDataFragment* DataFragment = EntityManagerPtr->GetFragmentDataPtr<FLocationDataFragment>(Handle);
	FVector2D OldCell = VectorToCell(DataFragment->OldLocation);
	Grid->RemoveAt(OldCell.X,OldCell.Y,Handle);
	EntityManagerPtr->Defer().DestroyEntity(Handle);
}

void UScaleSubsystem::DestroyStalledEntity()
{
	if (ToDestroy.Num() <= 0) return;
	FMassEntityHandle Handle = ToDestroy[0];
	ToDestroy.RemoveAt(0);
	
}




FVector2D UScaleSubsystem::VectorToCell(FVector Location)
{
	int32 X = static_cast<int32>(Location.X/CellSize);
	int32 Y = static_cast<int32>(Location.Y/CellSize);
	return FVector2D(X, Y);
}

TArray<FMassEntityHandle> UGrid::GetAt(int32 X, int32 Y)
{
	if (!XCells.Contains(X)) return TArray<FMassEntityHandle>();
	UGridCellY* CellY = XCells[X];
	if (!CellY) return TArray<FMassEntityHandle>();
	if (!CellY->YCells.Contains(Y)) return TArray<FMassEntityHandle>();
	UGridCell* Cell = CellY->YCells[Y];
	if (!Cell) return TArray<FMassEntityHandle>();
	return Cell->Handles;
}

TArray<FMassEntityHandle> UGrid::RemoveAt(int32 X, int32 Y, FMassEntityHandle ToRemoveHandle)
{
	if (!XCells.Contains(X)) return TArray<FMassEntityHandle>();
	UGridCellY* CellY = XCells[X];
	if (!CellY) return TArray<FMassEntityHandle>();
	if (!CellY->YCells.Contains(Y)) return TArray<FMassEntityHandle>();
	UGridCell* Cell = CellY->YCells[Y];
	if (!Cell) return TArray<FMassEntityHandle>();
	Cell->Handles.Remove(ToRemoveHandle);
	return Cell->Handles;
}

TArray<FMassEntityHandle> UGrid::GetAround(int32 X, int32 Y, int32 Radius)
{
	TArray<FMassEntityHandle> Handles = TArray<FMassEntityHandle>();
	for (int i = -Radius; i < Radius; ++i)
	{
		for (int j = -Radius; j < Radius; ++j)
		{
			Handles.Append(GetAt(X+i,Y+j));
		}
	}
	
	return Handles;
}

void UGrid::AddToGrid(int32 X, int32 Y, FMassEntityHandle Handle)
{
	if (!XCells.Contains(X)) XCells.Add(X, NewObject<UGridCellY>());
	UGridCellY* CellY = XCells[X];
	if (!CellY) CellY = NewObject<UGridCellY>();
	if (!CellY->YCells.Contains(Y)) CellY->YCells.Add(Y, NewObject<UGridCell>());
	UGridCell* Cell = CellY->YCells[Y];
	if (!Cell) Cell = NewObject<UGridCell>();
	Cell->Handles.Add(Handle);
}



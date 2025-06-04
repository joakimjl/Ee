// Fill out your copyright notice in the Description page of Project Settings.


#include "EeSubsystem.h"

#include <functional>

#include "CombatFragments.h"
#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassEntityBuilder.h"
#include "MassEntitySubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassEntityUtils.h"

void UEeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Initialize dependent subsystems before calling super
	Collection.InitializeDependency(UMassEntitySubsystem::StaticClass());
	
	Super::Initialize(Collection);
	
	EntityHandleGrid = TMap<int32, FEntityHandleGridCellY>();
	EeEntityManager = &GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	UE::Mass::FEntityBuilder Builder(*EeEntityManager);
	EntityBuilder = &Builder;
	
	// In here you can hook to delegates!
	// ie: OnFireHandle = FExample::OnFireDelegate.AddUObject(this, &UMyWorldSubsystem::OnFire);
}

void UEeSubsystem::Deinitialize()
{
	// In here you can unhook from delegates
	// ie: FExample::OnFireDelegate.Remove(OnFireHandle);
	EntityHandleGrid.Empty();
	ProjectileMeshComponents.Empty();
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
	if (!EntityHandleGrid.Contains(GridVec.X)) EntityHandleGrid.Add(GridVec.X, FEntityHandleGridCellY());
	if (!EntityHandleGrid[GridVec.X].InnerMap.Contains(GridVec.Y)) EntityHandleGrid[GridVec.X].InnerMap.Add(GridVec.Y, FGridCellData());
	if (!EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Contains(Handle)) EntityHandleGrid[GridVec.X].InnerMap[GridVec.Y].Handles.Add(Handle);

	return Location;
}

TArray<FMassEntityHandle> UEeSubsystem::EntitesAround(FIntVector2 InGrid, int32 SizeAround)
{
	TArray<FMassEntityHandle> Out;
	for (int32 i = -SizeAround; i <= SizeAround; i++)
	{
		for (int32 j = -SizeAround; j <= SizeAround; j++)
		{
			if (!EntityHandleGrid.Contains(InGrid.X+i)) continue;
			if (!EntityHandleGrid[InGrid.X+i].InnerMap.Contains(InGrid.Y+j)) continue;
			TArray<FMassEntityHandle>& Temp = EntityHandleGrid[InGrid.X+i].InnerMap[InGrid.Y+j].Handles;
			for (FMassEntityHandle Handle : Temp)
			{
				if (!EeEntityManager->IsEntityValid(Handle)) Temp.Remove(Handle);
			}
			Out.Append(EntityHandleGrid[InGrid.X+i].InnerMap[InGrid.Y+j].Handles);
		}
	}

	if (DebugEnable) UE_LOG(LogTemp, Warning, TEXT("Found %d entities around"), Out.Num());

	return Out;
}

bool UEeSubsystem::AttackLocation(FVector InLocation, int32 Area, int32 Team)
{
	FIntVector2 GridLoc = VectorToGrid(InLocation);
	TArray<FMassEntityHandle> AttackTargets = EntitesAround(GridLoc, Area);

	for (auto AttackTarget : AttackTargets)
	{
		FTransformFragment* TransformFrag = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(AttackTarget);
		if (EeEntityManager->GetFragmentDataPtr<FTransformFragment>(AttackTarget))
		{
			TransformFrag->GetMutableTransform().SetLocation(TransformFrag->GetTransform().GetLocation()+FVector(0,0,200.f));
		}
		SpawnProjectile(AttackTarget);
	}
	
	return true;
}


bool UEeSubsystem::SpawnProjectile(FMassEntityHandle Handle)
{
    FProjectileParams ProjectileParams = EeEntityManager->GetConstSharedFragmentDataChecked<FProjectileParams>(Handle);
    FProjectileVis ProjectileVis = EeEntityManager->GetSharedFragmentDataChecked<FProjectileVis>(Handle);

    // Find or create the ISM component for this mesh
    UInstancedStaticMeshComponent* TargetISM = nullptr;
    UStaticMesh* ProjectileMesh = ProjectileVis.ProjectileMesh.LoadSynchronous();
    
    // Find existing ISM for this mesh
    for (UInstancedStaticMeshComponent* ExistingISM : ProjectileMeshComponents)
    {
        if (ExistingISM && ExistingISM->GetStaticMesh() == ProjectileMesh)
        {
            TargetISM = ExistingISM;
            break;
        }
    }

    // Create new ISM if none exists for this mesh
    if (!TargetISM && GetWorld())
    {
        if (AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings())
        {
            TargetISM = NewObject<UInstancedStaticMeshComponent>(WorldSettings);
            TargetISM->RegisterComponent();
            TargetISM->SetMobility(EComponentMobility::Movable);
            TargetISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            TargetISM->bCastDynamicShadow = false;
            TargetISM->SetStaticMesh(ProjectileMesh);
            ProjectileMeshComponents.Add(TargetISM);
        }
    }

    if (TargetISM)
    {
        // Add instance to the correct ISM
        TargetISM->AddInstance(FTransform(), false);
        
        UE::Mass::FEntityBuilder Builder(*EeEntityManager);
        FProjectileParams& ProjectileParamsRef = Builder.Add_GetRef<FProjectileParams>();
        ProjectileParamsRef = std::ref(ProjectileParams);
        
        // Update ProjectileVis with the correct ISM
        FProjectileVis& ProjectileVisRef = Builder.Add_GetRef<FProjectileVis>();
        ProjectileVisRef = ProjectileVis;
        ProjectileVisRef.ProjectileMeshComponent = TargetISM;  // Use the correct ISM

        FTransform Transform = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(Handle)->GetTransform();
        Builder.Add_GetRef<FTransformFragment>().GetMutableTransform().SetTranslation(Transform.GetLocation());
        
        FProjectileFragment& ProjectileFragment = Builder.Add_GetRef<FProjectileFragment>();
        ProjectileFragment.Velocity = ProjectileParams.InitialSpeed * ProjectileParams.InitialDirection;
        
        Builder.Add<FProjectileTag>();
        Builder.Commit();
        
        UE_LOG(LogTemp, Warning, TEXT("Projectile spawned %s"), *ProjectileFragment.Velocity.ToString());
        return true;
    }

    return false;
}
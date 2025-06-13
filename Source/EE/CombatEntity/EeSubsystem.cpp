// Fill out your copyright notice in the Description page of Project Settings.


#include "EeSubsystem.h"

#include <functional>

#include "EeCombatFragments.h"
#include "EeStructs.h"
#include "GameplayTagContainer.h"
#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassEntityBuilder.h"
#include "MassEntitySubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassEntityUtils.h"
#include "MassStateTreeTypes.h"
#include "MassStateTreeSubsystem.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"

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
	for (UInstancedStaticMeshComponent* ISM : ProjectileMeshComponents)
	{
		ISM->DestroyComponent();
	}
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
	auto GridCopy = EntityHandleGrid; 
	for (int32 i = -SizeAround; i <= SizeAround; i++)
	{
		for (int32 j = -SizeAround; j <= SizeAround; j++)
		{
			if (!GridCopy.Contains(InGrid.X+i)) continue;
			if (!GridCopy[InGrid.X+i].InnerMap.Contains(InGrid.Y+j)) continue;
			TArray<FMassEntityHandle>& Temp = GridCopy[InGrid.X+i].InnerMap[InGrid.Y+j].Handles;
			for (FMassEntityHandle Handle : Temp)
			{
				if (!EeEntityManager->IsEntityValid(Handle)) Temp.Remove(Handle);
			}
			Out.Append(GridCopy[InGrid.X+i].InnerMap[InGrid.Y+j].Handles);
		}
	}

	if (DebugEnable) UE_LOG(LogTemp, Warning, TEXT("Found %d entities around"), Out.Num());

	return Out;
}

TArray<FMassEntityHandle> UEeSubsystem::EnemiesAround(FIntVector2 InGrid, int32 SizeAround, int32 Team)
{
	TArray<FMassEntityHandle> Entities = EntitesAround(InGrid, SizeAround);
	TArray<FMassEntityHandle> Out;
	for (FMassEntityHandle Entity : Entities)
	{
		if (!EeEntityManager->IsEntityValid(Entity)) continue;
		FTeamFragment* TeamFrag = EeEntityManager->GetFragmentDataPtr<FTeamFragment>(Entity);
		if (TeamFrag && TeamFrag->Team != Team) Out.Add(Entity);
	}
	return Out;
}

bool UEeSubsystem::AttackLocation(FVector InLocation, EDamageType DamageType, float Damage, float Area, int32 Team)
{
	FIntVector2 GridLoc = VectorToGrid(InLocation);
	int32 SizeAround = FMath::CeilToInt(Area/200)+1;
	TArray<FMassEntityHandle> AttackTargets = EntitesAround(GridLoc, SizeAround);

	for (auto AttackTarget : AttackTargets)
	{
		if (!EeEntityManager->IsEntityValid(AttackTarget)) continue; 
		FTeamFragment* TeamFrag = EeEntityManager->GetFragmentDataPtr<FTeamFragment>(AttackTarget);
		//if (TeamFrag) UE_LOG(LogTemp, Warning, TEXT("Team %d vs %d"), TeamFrag->Team, Team);
		if (TeamFrag && TeamFrag->Team == Team) continue;
		FTransformFragment* TransformFrag = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(AttackTarget);
		if (TransformFrag)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attacking %f is Distance with area: %f"), (TransformFrag->GetTransform().GetLocation()-InLocation).Size(),Area);
			if ((TransformFrag->GetTransform().GetLocation()-InLocation).Size() > Area) continue;
			FDamageFragment* DamageFrag = EeEntityManager->GetFragmentDataPtr<FDamageFragment>(AttackTarget);
			FVector ImpulseDir = TransformFrag->GetTransform().GetLocation() - InLocation + FVector(0,0,200);
			FVector ImpulseDirNorm = ImpulseDir.GetSafeNormal();
			float ImpulseSize = 100+ImpulseDir.Size()/10.f;
			//UE_LOG(LogTemp, Warning, TEXT("Impulse %s"), *ImpulseDir.ToString());
			if (DamageFrag)
			{
				if (!DamageFrag->DamageMap.Contains(DamageType)) DamageFrag->DamageMap.Add(DamageType, 0);
				FVector NewImpulse = ImpulseDirNorm*500.3f*Damage/ImpulseSize;
				NewImpulse.Z *= 2;
				DamageFrag->Impulse += NewImpulse;
				DamageFrag->DamageMap[DamageType] += Damage;
				//EeEntityManager->AddTagToEntity(AttackTarget,FDamageTag::StaticStruct());
				EeEntityManager->Defer().AddTag<FDamageTag>(AttackTarget);
				//EeEntityManager->Defer().PushCommand<FMassCommandAddTag<FDamageTag>>(AttackTarget);
				//UE_LOG(LogTemp, Warning, TEXT("Attacked %s"), *TransformFrag->GetTransform().GetLocation().ToString());
				GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntity(UE::Mass::Signals::StateTreeActivate,AttackTarget);
			}
		}
	}
	
	return true;
}


bool UEeSubsystem::SpawnProjectile(FMassEntityHandle Handle, FVector TargetLocation)
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
        ProjectileFragment.Velocity = ProjectileParams.InitialSpeed * ProjectileParams.InitialDirection * (Transform.GetRotation().Vector()-Transform.GetRotation().GetRightVector()+Transform.GetRotation().GetUpVector());
        
        Builder.Add<FProjectileTag>();
        Builder.Commit();
        
        //UE_LOG(LogTemp, Warning, TEXT("Projectile spawned %s"), *ProjectileFragment.Velocity.ToString());
        return true;
    }

    return false;
}

FTransform UEeSubsystem::GetEntityLocation(const FEeTargetData& EntityData)
{
	const FTransform Failed = FTransform::Identity; 
	const FMassEntityHandle Handle = FMassEntityHandle(EntityData.EntityNumber,EntityData.EntitySerial);
	if (!EeEntityManager->IsEntityValid(Handle)) return Failed;
	FTransformFragment* TransformFragPtr = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(Handle);
	if (TransformFragPtr) return TransformFragPtr->GetMutableTransform();
	return Failed;
};

bool UEeSubsystem::EntityIsValid(const  FEeTargetData& EntityData)
{
	return EeEntityManager->IsEntityValid(FMassEntityHandle(EntityData.EntityNumber,EntityData.EntitySerial));
}

bool UEeSubsystem::DestroyEntityWithData(const  FEeTargetData& EntityData)
{
	EeEntityManager->Defer().PushCommand<FMassCommandDestroyEntities>(FMassEntityHandle(EntityData.EntityNumber,EntityData.EntitySerial));
	return true;
}

void UEeSubsystem::DestroyEntityHandle(const FMassEntityHandle& Handle)
{
	EeEntityManager->Defer().PushCommand<FMassCommandDestroyEntities>(Handle);
}
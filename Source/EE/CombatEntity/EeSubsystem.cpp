// Fill out your copyright notice in the Description page of Project Settings.


#include "EeSubsystem.h"

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
	TArray<FMassEntityHandle> EntitiesProjectiles;
	//const UScriptStruct* FragmentTypes[] = { FTransformFragment::StaticStruct(), FProjectileParams::StaticStruct(), FProjectileFragment::StaticStruct() };
	//const FMassArchetypeHandle ArchetypeHandle = EeEntityManager->CreateArchetype(MakeArrayView(FragmentTypes, 3));
	//EeEntityManager->BatchCreateEntities(ArchetypeHandle, 1, EntitiesProjectiles);

	FProjectileParams ProjectileParams = EeEntityManager->GetConstSharedFragmentDataChecked<FProjectileParams>(Handle);
	FProjectileVis ProjectileVis = EeEntityManager->GetSharedFragmentDataChecked<FProjectileVis>(Handle);

	UE::Mass::FEntityBuilder Builder(*EeEntityManager);
	//UE::Mass::FEntityBuilder ProjectileBuilder = UE::Mass::FEntityBuilder(SharedRef);
	FProjectileParams& ProjectileParamsRef = Builder.Add_GetRef<FProjectileParams>();
	ProjectileParamsRef = ProjectileParams;
	FProjectileVis& ProjectileVisRef = Builder.Add_GetRef<FProjectileVis>();
	ProjectileVisRef = ProjectileVis;
	FTransform Transform = EeEntityManager->GetFragmentDataPtr<FTransformFragment>(Handle)->GetTransform();
	Builder.Add_GetRef<FTransformFragment>().GetMutableTransform().SetTranslation(Transform.GetLocation());
	FProjectileFragment& ProjectileFragment = Builder.Add_GetRef<FProjectileFragment>();
	ProjectileFragment.Velocity = ProjectileParams.InitialSpeed*ProjectileParams.InitialDirection;
	UE_LOG(LogTemp, Warning, TEXT("Projectile spawned %s"), *ProjectileFragment.Velocity.ToString());
	Builder.Add<FProjectileTag>();
	if (UWorld* World = GetWorld())
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			if (ProjectileMeshComponents.Contains(ProjectileVisRef.ProjectileMeshComponent) == false){
				UInstancedStaticMeshComponent* NewISM = NewObject<UInstancedStaticMeshComponent>(WorldSettings);
				NewISM->RegisterComponent();
				ProjectileMeshComponents.Add(NewISM);
				ProjectileVisRef.ProjectileMeshComponent = NewISM;
				NewISM->AddInstance(Transform, true);
				NewISM->SetStaticMesh(ProjectileVis.ProjectileMesh.LoadSynchronous());
				NewISM->SetWorldScale3D(FVector(1,1,1));
				NewISM->SetWorldRotation(FRotator(0,0,0));
				NewISM->SetWorldLocation(Transform.GetLocation());
				NewISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				NewISM->SetCollisionObjectType(ECC_WorldStatic);
				NewISM->SetCollisionResponseToAllChannels(ECR_Ignore);
				NewISM->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
				NewISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
			}
			else
			{
				ProjectileVisRef.ProjectileMeshComponent->AddInstance(Transform, true);
			}
		}
	}
	
	Builder.Commit();

	return true;
}

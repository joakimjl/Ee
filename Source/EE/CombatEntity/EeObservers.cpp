// Fill out your copyright notice in the Description page of Project Settings.

#include "EeObservers.h"
#include "CombatFragments.h"
#include "EeSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Components/InstancedStaticMeshComponent.h"

UProjectileObserverProcessor::UProjectileObserverProcessor()
: EntityQuery(*this)
{
	ObservedType = FProjectileFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Client);
}

void UProjectileObserverProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FProjectileFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FProjectileParams>(EMassFragmentPresence::All);
	EntityQuery.AddSharedRequirement<FProjectileVis>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FProjectileTag>(EMassFragmentPresence::All);
}

void UProjectileObserverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TArrayView<FProjectileFragment> ProjectileFragments = Context.GetMutableFragmentView<FProjectileFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		FProjectileVis& ProjectileVisIn = Context.GetMutableSharedFragment<FProjectileVis>();
		const FProjectileParams& ProjectileParams = Context.GetConstSharedFragment<FProjectileParams>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			ProjectileVisIn.ProjectileMeshComponent->AddInstance(FTransform(), true);
			FProjectileFragment& ProjectileFragment = ProjectileFragments[EntityIndex];
			ProjectileFragment.Velocity = ProjectileParams.InitialSpeed*ProjectileParams.InitialDirection;
		}
	});
}
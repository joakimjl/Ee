// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatProcessor.h"

#include "CombatFragments.h"
#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassNavigationFragments.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"


UCombatProcessor::UCombatProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetFragArr = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const FMassMovementParameters MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (true) continue;
		}
	});
}


UProjectileProcessor::UProjectileProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UProjectileProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddConstSharedRequirement<FProjectileParams>(EMassFragmentPresence::All);
	EntityQuery.AddSharedRequirement<FProjectileVis>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FProjectileFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FProjectileTag>(EMassFragmentPresence::All);
}

void UProjectileProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FProjectileFragment> ProjectileFragArr = Context.GetMutableFragmentView<FProjectileFragment>();
		FProjectileVis& ProjectileVisIn = Context.GetMutableSharedFragment<FProjectileVis>();
		const FProjectileParams& ProjectileParams = Context.GetConstSharedFragment<FProjectileParams>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = TransformFragArr[EntityIt].GetMutableTransform();
			FProjectileFragment& ProjectileFrag = ProjectileFragArr[EntityIt];
			MutableTransform.SetLocation(MutableTransform.GetLocation() + DeltaTime*ProjectileFrag.Velocity);
			//UE_LOG(LogTemp, Warning, TEXT("ProjectileFrag.Velocity: %s"), *ProjectileFrag.Velocity.ToString());
		}
	});
}

UDeathPhysicsProcessor::UDeathPhysicsProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UDeathPhysicsProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UDeathPhysicsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetFragArr = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const FMassMovementParameters MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (true) continue;
		}
	});
}
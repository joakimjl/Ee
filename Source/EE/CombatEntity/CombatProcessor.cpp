// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatProcessor.h"

#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassNavigationFragments.h"
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
			
			FMassMoveTargetFragment& MoveTargetFrag = MoveTargetFragArr[EntityIt];
			FTransformFragment& TransformFrag = TransformFragArr[EntityIt];
			FTransform CurrentTransform = TransformFrag.GetTransform();

			MoveTargetFrag.DistanceToGoal = (MoveTargetFrag.Center - CurrentTransform.GetLocation()).Size();
			if (MoveTargetFrag.DistanceToGoal <= MoveTargetFrag.SlackRadius+100.f)
			{
				MoveTargetFrag.Center = CurrentTransform.GetLocation()+FVector(FMath::RandRange(-1000.f,1000.f),FMath::RandRange(-1000.f,1000.f),0);
				MoveTargetFrag.DistanceToGoal = (MoveTargetFrag.Center - CurrentTransform.GetLocation()).Size();
				MoveTargetFrag.Forward = (MoveTargetFrag.Center - CurrentTransform.GetLocation()).GetSafeNormal();
				EMassMovementAction Action = MoveTargetFrag.GetCurrentAction();
			}
			UE_LOG(LogTemp, Display, TEXT("Distance to goal: %f"),MoveTargetFrag.DistanceToGoal);
		}
	});
}
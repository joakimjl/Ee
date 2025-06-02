// Fill out your copyright notice in the Description page of Project Settings.

#include "TransformProcessor.h"

#include "EulerTransform.h"
#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassNavigationFragments.h"
#include "Quaternion.h"
#include "Math/UnrealMathUtility.h"

#define EE_SPAWNABLE  ECC_GameTraceChannel2


UTransformProcessor::UTransformProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const FMassMovementParameters MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetArr = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = TransformFragArr[EntityIt].GetMutableTransform();
			FMassMoveTargetFragment& MoveTargetFrag = MoveTargetArr[EntityIt];
			FVector Loc = MutableTransform.GetLocation();
			if (MoveTargetFrag.Center.Z - Loc.Z <= 45.f) continue;
			if (MoveTargetFrag.DistanceToGoal <= 100.f) continue;
			FHitResult OutHit = FHitResult();
			bool Res = Context.GetWorld()->LineTraceSingleByChannel(OutHit,Loc + FVector::UpVector*150.f, Loc + FVector::UpVector*-150.f, EE_SPAWNABLE);
			if (Res)
			{
				FVector Up = OutHit.ImpactNormal;
				FVector Forward = MoveTargetFrag.Forward;

				FMatrix RotationMatrix = FRotationMatrix::MakeFromZX(Up,Forward);
				FQuat RotationQuat = RotationMatrix.ToQuat();
				
				float Alpha = 0.10f;
				
				FQuat SmoothedRotation = FQuat::Slerp(MutableTransform.GetRotation(), RotationQuat, Alpha);
				SmoothedRotation.Normalize();

				float SmoothedHeight = FMath::Lerp(MutableTransform.GetLocation().Z,OutHit.ImpactPoint.Z,Alpha*1.5f);
				FVector SmoothedLocation = FVector(MutableTransform.GetLocation().X, MutableTransform.GetLocation().Y, SmoothedHeight);

				MutableTransform.SetRotation(SmoothedRotation);
				MutableTransform.SetLocation(SmoothedLocation);
			}
		}
	});
}
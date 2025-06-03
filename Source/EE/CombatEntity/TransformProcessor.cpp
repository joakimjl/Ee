// Fill out your copyright notice in the Description page of Project Settings.

#include "TransformProcessor.h"

#include "CombatFragments.h"
#include "EeSubsystem.h"
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
	EntityQuery.AddRequirement<FTransformDistChecker>(EMassFragmentAccess::ReadWrite);
}

void UTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
	UEeSubsystem* EeSubsystem = Context.GetWorld()->GetSubsystem<UEeSubsystem>();

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime, EeSubsystem](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const FMassMovementParameters MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();
		const TArrayView<FTransformDistChecker> DistCheckers = Context.GetMutableFragmentView<FTransformDistChecker>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetArr = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = TransformFragArr[EntityIt].GetMutableTransform();
			FTransformDistChecker& DistChecker = DistCheckers[EntityIt];
			FMassMoveTargetFragment& MoveTargetFrag = MoveTargetArr[EntityIt];
			FVector Loc = MutableTransform.GetLocation();
			if (abs(MoveTargetFrag.Center.Z - Loc.Z) >= 5.f) DistChecker.FullyLerped = true;
			//if (MoveTargetFrag.DistanceToGoal <= 100.f) continue;
			DistChecker.DistSinceZCheck = (DistChecker.LastLocation - Loc).Size();
			//if (DistChecker.DistSinceZCheck <= 100.f) continue;
			FHitResult OutHit = FHitResult();

			float Alpha = 2.55f*DeltaTime;
			
			if (!DistChecker.FullyLerped)
			{
				FQuat SmoothedRotation = FQuat::Slerp(MutableTransform.GetRotation(), DistChecker.TargetRotation, Alpha);
				SmoothedRotation.Normalize();

				float SmoothedHeight = FMath::Lerp(MutableTransform.GetLocation().Z, DistChecker.TargetHeight,Alpha);
				FVector SmoothedLocation = FVector(MutableTransform.GetLocation().X, MutableTransform.GetLocation().Y, SmoothedHeight);

				MutableTransform.SetRotation(SmoothedRotation);
				MutableTransform.SetLocation(SmoothedLocation);
				//If abs of z and abs of smoothedLocationZ is +-0.5
				if ((MutableTransform.GetLocation().Z < DistChecker.TargetHeight + 0.5f
					|| MutableTransform.GetLocation().Z > DistChecker.TargetHeight - 0.5f)
					&& SmoothedRotation.AngularDistance(DistChecker.TargetRotation) <= 0.01)
				{
					DistChecker.FullyLerped = true;
				}
				continue;
			}
			bool Res = Context.GetWorld()->LineTraceSingleByChannel(OutHit,Loc + FVector::UpVector*250.f, Loc + FVector::UpVector*-250.f, EE_SPAWNABLE);
			if (Res)
			{
				FVector Up = OutHit.ImpactNormal;
				FVector Forward = MoveTargetFrag.Forward;

				FMatrix RotationMatrix = FRotationMatrix::MakeFromZX(Up,Forward);
				FQuat RotationQuat = RotationMatrix.ToQuat();

				DistChecker.TargetRotation = RotationQuat;
				DistChecker.TargetHeight = OutHit.ImpactPoint.Z;
				DistChecker.FullyLerped = false;
				
				FQuat SmoothedRotation = FQuat::Slerp(MutableTransform.GetRotation(), RotationQuat, Alpha);
				SmoothedRotation.Normalize();

				float SmoothedHeight = FMath::Lerp(MutableTransform.GetLocation().Z,OutHit.ImpactPoint.Z,Alpha*1.5f);
				FVector SmoothedLocation = FVector(MutableTransform.GetLocation().X, MutableTransform.GetLocation().Y, SmoothedHeight);

				MutableTransform.SetRotation(SmoothedRotation);
				MutableTransform.SetLocation(SmoothedLocation);
				DistChecker.DistSinceLocMark = 0;
				const FVector NewLast = EeSubsystem->MoveSelfInGrid(
					Context.GetEntity(EntityIt),DistChecker.LastLocation);
				DistChecker.LastLocation = NewLast;
				DistChecker.DistSinceZCheck = 0;
			}
		}
	});
}
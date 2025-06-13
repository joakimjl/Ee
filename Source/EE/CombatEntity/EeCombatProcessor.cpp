// Fill out your copyright notice in the Description page of Project Settings.

#include "EeCombatProcessor.h"

#include "EeCombatFragments.h"
#include "EeSubsystem.h"
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
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = TransformFragArr[EntityIt].GetMutableTransform();
			FProjectileFragment& ProjectileFrag = ProjectileFragArr[EntityIt];
			float TempWeight = 10.f;
			ProjectileFrag.Velocity = ProjectileFrag.Velocity*(1 - 0.01f*DeltaTime) + TempWeight*FVector(0.f,0.f,-98.f)*DeltaTime;
			MutableTransform.SetLocation(MutableTransform.GetLocation() + DeltaTime*ProjectileFrag.Velocity);
			if (MutableTransform.GetLocation().Z <= 215.f)
			{
				MutableTransform.SetLocation(MutableTransform.GetLocation() + FVector(0.f, 0.f, ProjectileFrag.Velocity.Z*+0.5f));
				ProjectileFrag.Velocity.Z = ProjectileFrag.Velocity.Z*-0.5f;
			}
		}
	});
}

UProjectileInstanceUpdateProcessor::UProjectileInstanceUpdateProcessor()
	: EntityQuery(*this)
{
	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::ApplyForces;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

}

void UProjectileInstanceUpdateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSharedRequirement<FProjectileVis>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FProjectileTag>(EMassFragmentPresence::All);
}

void UProjectileInstanceUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("ProjectileInstanceUpdate"));
	int32 CurNum = 0;
	int32* NumPtr = &CurNum;

	EntityQuery.ForEachEntityChunk(Context, [this, NumPtr](FMassExecutionContext& Context)
	{
		FProjectileVis& ProjectileVis = Context.GetMutableSharedFragment<FProjectileVis>();
		TConstArrayView<FTransformFragment> TransformFragArr = Context.GetFragmentView<FTransformFragment>();
        
		//if (!ProjectileVis.ProjectileMeshComponent || !ProjectileVis.ProjectileMeshComponent->IsValidLowLevel())
		//{
		//	return;
		//}

		TArray<FTransform> Transforms = TArray<FTransform>();
		Transforms.Reserve(Context.GetNumEntities());

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			Transforms.Add(TransformFragArr[EntityIndex].GetTransform());
		}

		if (Transforms.Num() > 0)
		{
			ProjectileVis.ProjectileMeshComponent->BatchUpdateInstancesTransforms(*NumPtr, Transforms, true, true, true);
		}
		*NumPtr += Context.GetNumEntities();
	});
}


UDeathPhysicsProcessor::UDeathPhysicsProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::ApplyForces);
}

void UDeathPhysicsProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FDeadFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FDeadTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<UEeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UDeathPhysicsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FDeadFragment> DeadFragArr = Context.GetMutableFragmentView<FDeadFragment>();
		UEeSubsystem& EeSubsystemIn = Context.GetMutableSubsystemChecked<UEeSubsystem>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = TransformFragArr[EntityIt].GetMutableTransform();
			FDeadFragment& DeadFrag = DeadFragArr[EntityIt];
			DeadFrag.Velocity = DeadFrag.Velocity*(1 - 0.1f*DeltaTime)-DeadFrag.Weight*FVector(0.f, 0.f, 98.f)*DeltaTime;
			MutableTransform.SetLocation(MutableTransform.GetLocation() + DeltaTime*DeadFrag.Velocity);
			MutableTransform.SetRotation((-DeadFrag.Velocity).ToOrientationQuat());
			if (MutableTransform.GetLocation().Z <= -215.f)
			{
				EeSubsystemIn.DestroyEntityHandle(Context.GetEntity(EntityIt));
			}
		}
	});
}
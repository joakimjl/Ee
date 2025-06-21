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
	EntityQuery.AddRequirement<FOffensiveStatsBase>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		
		const TArrayView<FTransformFragment> TransformFragArr = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetFragArr = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassMoveTargetFragment> OffensiveStatsBase = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
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


UAttackCooldownProcessor::UAttackCooldownProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UAttackCooldownProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FOffensiveStatsBase>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FOffensiveStatsParams>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHasCooldownTag>(EMassFragmentPresence::All);
}

void UAttackCooldownProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		const TArrayView<FOffensiveStatsBase> OffensiveStatsBaseArr = Context.GetMutableFragmentView<FOffensiveStatsBase>();
		const FOffensiveStatsParams OffensiveStatsParams = Context.GetConstSharedFragment<FOffensiveStatsParams>();
		//UE_LOG(LogTemp, Display, TEXT("Checking cooldown"));

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FOffensiveStatsBase& OffensiveStatsBase = OffensiveStatsBaseArr[EntityIt];
			OffensiveStatsBase.TimeUntilAttack -= DeltaTime;
			//FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
			//if (OffensiveStatsBase.TimeUntilAttack <= 0.f) EntityManager.Defer().RemoveTag<FHasCooldownTag>(Context.GetEntity(EntityIt));
		}
	});
}



UCollisionProcessor::UCollisionProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FWeightFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddSubsystemRequirement<UEeSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		UEeSubsystem& EeSubsystem = Context.GetMutableSubsystemChecked<UEeSubsystem>();
		const TArrayView<FTransformFragment> FTransformFragmentArr = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadiusArr = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FWeightFragment> WeightArr = Context.GetFragmentView<FWeightFragment>();
		const TArrayView<FCollisionFragment> CollisionFragmentArr = Context.GetMutableFragmentView<FCollisionFragment>();

		FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& MutableTransform = FTransformFragmentArr[EntityIt].GetMutableTransform();
			const FAgentRadiusFragment& AgentRadius = AgentRadiusArr[EntityIt];
			const FWeightFragment& Weight = WeightArr[EntityIt];
			FIntVector2 GridLoc = EeSubsystem.VectorToGrid(MutableTransform.GetLocation());
			FCollisionFragment& CollisionFragment = CollisionFragmentArr[EntityIt];
			TArray<FMassEntityHandle> EntityHandles = CollisionFragment.CollisionEntities;
			for (FMassEntityHandle EntityHandle : EntityHandles)
			{
				if (EntityHandle == Context.GetEntity(EntityIt)) continue;
				float EntityRadius = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(EntityHandle)->Radius;
				float EntityWeight = EntityManager.GetFragmentDataPtr<FWeightFragment>(EntityHandle)->Weight;
				FTransform& EntityTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(EntityHandle)->GetMutableTransform();
				FVector EntityLocation = EntityTransform.GetLocation();
				float Distance = FVector::Dist(MutableTransform.GetLocation(), EntityLocation);
				if (Distance <= AgentRadius.Radius + EntityRadius)
				{
					float WeightImpact = FMath::Max(FMath::Min(1.f*(Weight.Weight/EntityWeight),0.01),10.f);
					WeightImpact *= 10.0f;
					FVector Push = (EntityLocation - MutableTransform.GetLocation());
					Push.Z = 0.f;
					EntityTransform.SetLocation(EntityLocation + Push.GetSafeNormal()*WeightImpact*DeltaTime);
				}
			}
			CollisionFragment.TimerUntilRecheck -= DeltaTime;
			if (CollisionFragment.TimerUntilRecheck > 0.f) continue;
			float Random = FMath::RandRange(0.f, 0.3f);
			CollisionFragment.TimerUntilRecheck = 0.2f+Random;
			CollisionFragment.CollisionEntities = EeSubsystem.EntitesAround(GridLoc,1+static_cast<int32>(AgentRadius.Radius/EeSubsystem.GetGridSize()));
		}
	});
}
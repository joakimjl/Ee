// Fill out your copyright notice in the Description page of Project Settings.

#include "EeObservers.h"
#include "CombatFragments.h"
#include "EeSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
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

	EntityQuery.RegisterWithProcessor(*this);
}

void UProjectileObserverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE_LOG(LogTemp, Warning, TEXT("Projectile Tag Observed"));
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


UDamageObserverProcessor::UDamageObserverProcessor()
: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	//ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
	bRequiresGameThreadExecution = true;
}

void UDamageObserverProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FDefenceStatsBase>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FDamageFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FDamageTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FDeadTag>(EMassFragmentPresence::None);

	EntityQuery.RegisterWithProcessor(*this);
}

void UDamageObserverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	//UE_LOG(LogTemp, Warning, TEXT("Damage Tag Observed"));
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		TArrayView<FDefenceStatsBase> DefenceStatsBaseArr = Context.GetMutableFragmentView<FDefenceStatsBase>();
		TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FDamageFragment> DamageFragmentArr = Context.GetMutableFragmentView<FDamageFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FDefenceStatsBase& DefenceStats = DefenceStatsBaseArr[EntityIndex];
			FTransform& TransformFragment = TransformFragments[EntityIndex].GetMutableTransform();
			FDamageFragment& DamageFragment = DamageFragmentArr[EntityIndex];
			//UE_LOG(LogTemp, Warning, TEXT("Damage Entity"));

			//float DamageImpulse = 0.f;

			TArray<EDamageType> Keys;
			DamageFragment.DamageMap.GenerateKeyArray(Keys);
			for (const auto& Key : Keys)
			{
				DefenceStats.CurHealth -= DamageFragment.DamageMap[Key];
				//DamageImpulse += DamageFragment.DamageMap[Key];
				DamageFragment.DamageMap[Key] = 0;
			}
			if (DefenceStats.CurHealth <= 0)
			{
				DefenceStats.CurHealth = 0;
				TransformFragment.SetLocation(TransformFragment.GetLocation() + FVector(0.f, 0.f, 50.f));
				EntityManager.Defer().PushCommand<FMassCommandAddTag<FDeadTag>>(Context.GetEntity(EntityIndex));
				FDeadFragment* DeadFrag = EntityManager.GetFragmentDataPtr<FDeadFragment>(Context.GetEntity(EntityIndex));
				//Random X and Y, Z always set amount
				if (DeadFrag)
				{
					DeadFrag->Velocity = DamageFragment.Impulse + FVector(0,0, 999550.f/DamageFragment.Impulse.Size());
				}
			}
			DamageFragment.Impulse = FVector(0.f, 0.f, 0.f);
			EntityManager.Defer().PushCommand<FMassCommandRemoveTag<FDamageTag>>(Context.GetEntity(EntityIndex));
		}
	});
}
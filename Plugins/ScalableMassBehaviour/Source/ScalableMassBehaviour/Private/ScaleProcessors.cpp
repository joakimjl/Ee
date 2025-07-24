// Fill out your copyright notice in the Description page of Project Settings.


#include "ScaleProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "ScaleFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationProcessor.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "ScaleSubsystem.h"
#include "Animation/AnimData/AnimDataModel.h"

UAnimationProcessor::UAnimationProcessor()
	:EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FVertexAnimations>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();
	TimeAccumulator = GetWorld()->GetTimeSeconds();
	//TArray<FString> MeshNames = AnimationSubsystem.GetMeshNames();
	UMassCrowdRepresentationSubsystem* RepresentationSubsystem = Context.GetWorld()->GetSubsystem<UMassCrowdRepresentationSubsystem>();
	TArray<FString> MeshNames;
	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime, RepresentationSubsystem](FMassExecutionContext& Context)
	{
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationFragmentArrayView = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		FMassInstancedStaticMeshInfoArrayView ISMInfosView = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		TArrayView<FAnimationFragment> AnimationFragmentArrayView = Context.GetMutableFragmentView<FAnimationFragment>();
		FVertexAnimations VertFrag = Context.GetSharedFragment<FVertexAnimations>();
		
		TArray<EAnimationState> Ordering = TArray<EAnimationState>();
		Ordering.Add(EAnimationState::Idle);
		Ordering.Add(EAnimationState::Attacking);
		Ordering.Add(EAnimationState::Running);
		Ordering.Add(EAnimationState::Dead);

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FAnimationFragment& AnimationFragment = AnimationFragmentArrayView[EntityIndex];
			FMassRepresentationFragment& RepresentationFragment = RepresentationFragmentArrayView[EntityIndex];
			if (!ISMInfosView.IsValidIndex(RepresentationFragment.StaticMeshDescHandle.ToIndex())) continue;
			FMassInstancedStaticMeshInfo ISMInfo = ISMInfosView[RepresentationFragment.StaticMeshDescHandle.ToIndex()];
			
			const FString CurMeshName = ISMInfo.GetDesc().Meshes[0].Mesh->GetName();
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODList[EntityIndex];
			
			float CumulativeFrames = 0;
			float StartFrame = 0;
			float EndFrame = 0;
			float Framerate = 0;

			//TODO Separate channel for Damage received animation and fragment shader (Fire Ice...).

			if (AnimationFragment.LerpAlpha > 0)
			{
				AnimationFragment.LerpAlpha = FMath::Clamp(AnimationFragment.LerpAlpha-DeltaTime*3.f,0.f,1.f);
			}
			for (int i = 0; i < Ordering.Num(); ++i)
			{
				auto Key = Ordering[i];
				if (!VertFrag.AnimSequences.Contains(Key)) continue;
				auto EleAnim = VertFrag.AnimSequences[Key];
				int32 CurrentMaxFrame = EleAnim->GetNumberOfSampledKeys();
				if (AnimationFragment.CurrentState == Key)
				{
					StartFrame = CumulativeFrames;
					EndFrame = CumulativeFrames+CurrentMaxFrame-1;
					Framerate = EleAnim->GetSamplingFrameRate().AsDecimal();
				}
				CumulativeFrames += CurrentMaxFrame;
			}
			AnimationFragment.CurrentAnimationFrame += AnimationFragment.AnimationSpeed*DeltaTime*Framerate;
			
			if (AnimationFragment.CurrentState != AnimationFragment.PreviousState && AnimationFragment.LerpAlpha <= 0.f)
			{
				AnimationFragment.LerpAlpha = 1.f;
				AnimationFragment.PreviousAnimationFrame = AnimationFragment.CurrentAnimationFrame;
				AnimationFragment.CurrentAnimationFrame = 0;
				AnimationFragment.PreviousState = AnimationFragment.CurrentState;
			}
			if (AnimationFragment.LerpAlpha <= 0.f)
			{
				AnimationFragment.PrevStart = StartFrame;
				AnimationFragment.PrevEnd = EndFrame;
			}
			if (RepresentationFragment.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				float CurrentFrame = StartFrame+FMath::Modulo(AnimationFragment.CurrentAnimationFrame,EndFrame-StartFrame);
				float PreviousFrame = AnimationFragment.PrevStart+FMath::Modulo(AnimationFragment.PreviousAnimationFrame,AnimationFragment.PrevEnd-AnimationFragment.PrevStart);
				ISMInfo.AddBatchedCustomDataFloats({CurrentFrame,
					PreviousFrame,
					AnimationFragment.LerpAlpha,
					AnimationFragment.AnimationUnitScale},
					RepresentationLOD.LODSignificance, RepresentationLOD.PrevLOD);
			}
		}
	});
}

URegisterProcessor::URegisterProcessor()
	:EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void URegisterProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	//FMassEntityQuery EntityQuery(EntityManager);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FLocationDataFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UScaleSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URegisterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	float DeltaTime = FMath::Min(Context.GetDeltaTimeSeconds(),0.2f);

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		UScaleSubsystem& ScaleSubsystem = Context.GetMutableSubsystemChecked<UScaleSubsystem>();
		TArrayView<FTransformFragment> TransformFragmentArrayView = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FLocationDataFragment> LocationDataFragmentArrayView = Context.GetMutableFragmentView<FLocationDataFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FLocationDataFragment& LocationDataFragment = LocationDataFragmentArrayView[EntityIndex];
			LocationDataFragment.TimeSince += DeltaTime;
			if (LocationDataFragment.TimeSince < LocationDataFragment.BaseRefresh) continue;
			FVector Location = TransformFragmentArrayView[EntityIndex].GetTransform().GetLocation();
			FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);
			FVector NewOldLocation = ScaleSubsystem.RegisterToGrid(Location,
				EntityHandle,
				LocationDataFragment.OldLocation);
			LocationDataFragment.OldLocation = NewOldLocation;
			LocationDataFragment.TimeSince = 0.f;
		}
	});
}



UCollisionProcessor::UCollisionProcessor()
	:EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	//FMassEntityQuery EntityQuery(EntityManager);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollisionDataFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UScaleSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	float DeltaTime = FMath::Min(Context.GetDeltaTimeSeconds(),0.2f);

	EntityQuery.ForEachEntityChunk(Context, [this, DeltaTime](FMassExecutionContext& Context)
	{
		UScaleSubsystem& ScaleSubsystem = Context.GetMutableSubsystemChecked<UScaleSubsystem>();
		TArrayView<FTransformFragment> TransformFragmentArrayView = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FCollisionDataFragment> CollisionDataFragmentArrayView = Context.GetMutableFragmentView<FCollisionDataFragment>();
		TArrayView<FAgentRadiusFragment> AgentRadiusFragmentArrayView = Context.GetMutableFragmentView<FAgentRadiusFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FCollisionDataFragment& CollisionDataFragment = CollisionDataFragmentArrayView[EntityIndex];
			FTransform& MutableTransform = TransformFragmentArrayView[EntityIndex].GetMutableTransform();
			FVector Location = MutableTransform.GetLocation();
			FAgentRadiusFragment& AgentRadiusFragment = AgentRadiusFragmentArrayView[EntityIndex];
			
			CollisionDataFragment.TimeSinceLastCheck += DeltaTime*FMath::RandRange(0.9f,1.1f);
			if (!(CollisionDataFragment.TimeSinceLastCheck <= CollisionDataFragment.CheckDelay))
			{
				TArray<FMassEntityHandle> Handles = ScaleSubsystem.GetNumberClosestEntities(Location,
					AgentRadiusFragment.Radius*1.3f,
					CollisionDataFragment.MaxEntitiesToCheck);
				//UE_LOG(LogTemp, Warning, TEXT("Closest Entities: %d"), Handles.Num());
				CollisionDataFragment.ClosestEntities = Handles;
				CollisionDataFragment.TimeSinceLastCheck = 0;
			}
			FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
			for (FMassEntityHandle Handle : CollisionDataFragment.ClosestEntities)
			{
				
				if (!EntityManager.IsEntityValid(Handle)) continue;
				FTransformFragment* TransformFrag = EntityManager.GetFragmentDataPtr<FTransformFragment>(Handle);
				if (!TransformFrag) continue;
				FVector OtherLocation = TransformFrag->GetTransform().GetLocation();
				float Distance = (Location - OtherLocation).Size();
				if (Distance <= AgentRadiusFragment.Radius)
				{
					FCollisionDataFragment* OtherCollisionData = EntityManager.GetFragmentDataPtr<FCollisionDataFragment>(Handle);
					if (!OtherCollisionData) continue;
					float WeightMulti = 0.5f + OtherCollisionData->CollisionMass / CollisionDataFragment.CollisionMass;
					FVector SelfPushed = (Location - OtherLocation).GetSafeNormal() * WeightMulti * DeltaTime*30.f;
					SelfPushed.Z = 0.f;
					//UE_LOG(LogTemp, Warning, TEXT("Pushing: %s"), *SelfPushed.ToString());
					MutableTransform.SetLocation(MutableTransform.GetLocation() + SelfPushed);
				}
			}
		}
	});
}

/*
UScaleProcessors::UScaleProcessors()
	:EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UScaleProcessors::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	//FMassEntityQuery EntityQuery(EntityManager);

	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
	
}

void UScaleProcessors::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	float DeltaTime = FMath::Min(Context.GetDeltaTimeSeconds(),0.2f);
	
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		
	});
}
*/

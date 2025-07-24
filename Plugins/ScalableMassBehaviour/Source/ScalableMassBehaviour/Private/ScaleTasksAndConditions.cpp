// Fill out your copyright notice in the Description page of Project Settings.

#include "ScaleTasksAndConditions.h"
#include "Engine/World.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "ScaleSubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeTypes.h"
#include "MassNavigationFragments.h"
#include "ScaleFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassRepresentationProcessor.h"
#include "MassCrowdRepresentationSubsystem.h"

FGetRandomLocationInRange::FGetRandomLocationInRange()
{
	bShouldCallTick = false;
}

bool FGetRandomLocationInRange::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkExternalData(MassSignalSubsystemHandle);
	//Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(AnimationFragmentHandle);
	Linker.LinkExternalData(DeathFragmentHandle);
	return true;
}

void FGetRandomLocationInRange::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(AnimationFragmentHandle);
	Builder.AddReadWrite(DeathFragmentHandle);
}

EStateTreeRunStatus FGetRandomLocationInRange::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	//const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	FVector RandomLocation;
	if (InstanceData.Radius <= 0.f)
	{
		RandomLocation = InstanceData.InLocation;
	} else {
		FVector NewLoc = InstanceData.InLocation;
		NewLoc.X += FMath::FRandRange(-InstanceData.Radius, InstanceData.Radius);
		NewLoc.Y += FMath::FRandRange(-InstanceData.Radius, InstanceData.Radius);
		RandomLocation = NewLoc;
	}

	FMassTargetLocation OutLocation = FMassTargetLocation();
	OutLocation.EndOfPathPosition = RandomLocation;
	OutLocation.EndOfPathIntent = EMassMovementAction::Stand;

	InstanceData.TargetLocation = OutLocation;

	FAnimationFragment& AnimFrag = Context.GetExternalData(AnimationFragmentHandle);
	AnimFrag.CurrentState = EAnimationState::Running;
	
	return EStateTreeRunStatus::Running;
}


FGetProcessableLocation::FGetProcessableLocation()
{
	bShouldCallTick = false;
}

bool FGetProcessableLocation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	return true;
}

void FGetProcessableLocation::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadOnly(EntityTransformHandle);
}

EStateTreeRunStatus FGetProcessableLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	//const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	FVector EntityLocation = TransformFragment.GetTransform().GetLocation();
	FMassEntityHandle ClosestHandle = ScaleSubsystem.GetClosestResource(InstanceData.ResourceType, EntityLocation);
	FVector WalkLocation;
	if (ClosestHandle.IsValid() == false) {
		TArray<FVector> ResourceLocations = ScaleSubsystem.GetResources(InstanceData.ResourceType);
		WalkLocation = ResourceLocations[0];
	} else
	{
		WalkLocation = ScaleSubsystem.GetEntityLocation(ClosestHandle);
	}

	FMassTargetLocation OutLocation = FMassTargetLocation();
	OutLocation.EndOfPathPosition = WalkLocation;
	OutLocation.EndOfPathIntent = EMassMovementAction::Stand;

	InstanceData.TargetLocation = OutLocation;
	
	return EStateTreeRunStatus::Running;
}

bool FNavDoneCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	bool Rebuilt = MassStateTreeContext.GetWorld()->IsNavigationRebuilt();

	return Rebuilt^bInvert;
}


bool FIsCloseEnoughCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);

	return true;
}

bool FIsCloseEnoughCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	const FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	FVector EntityLocation = TransformFragment.GetTransform().GetLocation();
	
	float Distance;
	if (bUseMassTargetLocation)
	{
		Distance = (InstanceData.InMassLocation.EndOfPathPosition.Get(FVector::ZeroVector) - EntityLocation).Size();
	} else
	{
		Distance = (InstanceData.InLocation - EntityLocation).Size();
	}
	bool bIsCloseEnough = Distance < InstanceData.AcceptableDistance;

	return bIsCloseEnough^bInvert;
}

FProcessResource::FProcessResource()
{
	bShouldCallTick = true;
}

bool FProcessResource::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(ResourceFragmentHandle);
	Linker.LinkExternalData(VelocityFragmentHandle);
	Linker.LinkExternalData(DesiredFragmentHandle);
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FProcessResource::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(EntityTransformHandle);
	Builder.AddReadWrite(ResourceFragmentHandle);
	Builder.AddReadWrite(VelocityFragmentHandle);
	Builder.AddReadWrite(DesiredFragmentHandle);
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadWrite(MassSignalSubsystemHandle);
}

EStateTreeRunStatus FProcessResource::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FMassVelocityFragment& VelocityFragment = Context.GetExternalData(VelocityFragmentHandle);
	VelocityFragment.Value = FVector::ZeroVector;
	FMassDesiredMovementFragment& DesiredFragment = Context.GetExternalData(DesiredFragmentHandle);
	DesiredFragment.DesiredVelocity = FVector::ZeroVector;

	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), InstanceData.ProcessDuration);


	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FProcessResource::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FResourceFragment& ResourceFragment = Context.GetExternalData(ResourceFragmentHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	
	if (DeltaTime >= InstanceData.ProcessDuration)
	{
		const EProcessable Resource = InstanceData.ResourceType;

		if (InstanceData.TaskType == ETaskType::Process)
		{
			if (ScaleSubsystem.ReqMap.ReqMap.Contains(Resource))
			{
				FProcessableReqArr ProcessableArr = ScaleSubsystem.ReqMap.ReqMap[Resource];
				for (int i = 0; i < ScaleSubsystem.ReqMap.ReqMap[Resource].TypeArr.Num(); ++i)
				{
					if (ResourceFragment.Carrying[ProcessableArr.TypeArr[i]] < ProcessableArr.AmountArr[i])
						return EStateTreeRunStatus::Failed;
				}
			}
		}

		if (InstanceData.TaskType == ETaskType::Pickup)
		{
			if (!ResourceFragment.BonusMap.Contains(Resource))
				ResourceFragment.BonusMap.Add(Resource, 1.f);
			if (!ResourceFragment.Carrying.Contains(Resource))
			{
				ResourceFragment.Carrying.Add(Resource, 1*ResourceFragment.BonusMap[Resource]);
			} else
			{
				ResourceFragment.Carrying[Resource] += 1*ResourceFragment.BonusMap[Resource];
			}
			return EStateTreeRunStatus::Succeeded;
		}

		if (InstanceData.TaskType == ETaskType::Drop)
		{
			FVector EntityLocation = TransformFragment.GetTransform().GetLocation();
			FMassEntityHandle Target = ScaleSubsystem.GetClosestResource(Resource, EntityLocation);
			if (Target.IsValid())
			{
				TArray<EProcessable> Keys = TArray<EProcessable>();
				ResourceFragment.Carrying.GetKeys(Keys);
				for (int i = 0; i < Keys.Num(); ++i)
				{
					const EProcessable CurResource = Keys[i];
					if (Keys[i] == EProcessable::Storage) continue;
					const int32 CarryingAmount = ResourceFragment.Carrying[CurResource];
					ResourceFragment.Carrying[CurResource] = 0;
					bool Res = ScaleSubsystem.AddToEntity(Target, CurResource, CarryingAmount);
				}
				return EStateTreeRunStatus::Succeeded;
			}
			else
			{
				FMassEntityHandle Handle = ScaleSubsystem.RegisteredResources[InstanceData.ResourceType].Handles[0];
				if (Handle.IsValid())
				{
					ScaleSubsystem.AddToEntity(Handle, Resource, 1);
					return EStateTreeRunStatus::Succeeded;
				}
			}
			return EStateTreeRunStatus::Failed;
		}
	}
	
	return EStateTreeRunStatus::Running;
}




FFindClosestEnemy::FFindClosestEnemy()
{
	bShouldCallTick = true;
}

bool FFindClosestEnemy::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(AttackFragmentHandle);
	Linker.LinkExternalData(TeamFragmentHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FFindClosestEnemy::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(EntityTransformHandle);
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadWrite(AttackFragmentHandle);
	Builder.AddReadWrite(TeamFragmentHandle);
	Builder.AddReadWrite(MassSignalSubsystemHandle);
}

EStateTreeRunStatus FFindClosestEnemy::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAttackFragment& AttackFragment = Context.GetExternalData(AttackFragmentHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	FTeamFragment& TeamFragment = Context.GetExternalData(TeamFragmentHandle);

	InstanceData.FoundEntityTarget = true;
	FSmbEntityData EntityData = ScaleSubsystem.GetClosestEnemy(TransformFragment.GetTransform().GetLocation(), TeamFragment.TeamID, InstanceData.MaxDistance);
	if (EntityData.SerialNumber == -1 && EntityData.Index == -1) InstanceData.FoundEntityTarget = false;
	//UE_LOG(LogTemp, Display, TEXT("Found entity was (Enter): %i %i which means: %i"), EntityData.Index, EntityData.SerialNumber, InstanceData.FoundEntityTarget);

	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	if (EntityData.SerialNumber == -1 && EntityData.Index == -1)
	{
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
			UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), FMath::RandRange(0.01,0.1));
	}

	if (bTickingFind)
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
			UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), TickDelay + FMath::RandRange(0.01,0.1));

	InstanceData.EntityTarget = EntityData;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FFindClosestEnemy::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAttackFragment& AttackFragment = Context.GetExternalData(AttackFragmentHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	FTeamFragment& TeamFragment = Context.GetExternalData(TeamFragmentHandle);

	InstanceData.FoundEntityTarget = true;
	FSmbEntityData EntityData = ScaleSubsystem.GetClosestEnemy(TransformFragment.GetTransform().GetLocation(), TeamFragment.TeamID, InstanceData.MaxDistance);
	if (EntityData.SerialNumber == -1 && EntityData.Index == -1) InstanceData.FoundEntityTarget = false;
	//UE_LOG(LogTemp, Display, TEXT("Found entity was (Tick): %i %i which means: %i"), EntityData.Index, EntityData.SerialNumber, InstanceData.FoundEntityTarget);

	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	if (bTickingFind)
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
			UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), TickDelay + FMath::RandRange(0.01,0.1));
	
	InstanceData.EntityTarget = EntityData;
	
	return EStateTreeRunStatus::Running;
}



FProcessAttack::FProcessAttack()
{
	bShouldCallTick = true;
}

bool FProcessAttack::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(AnimationFragmentHandle);
	Linker.LinkExternalData(AttackFragmentHandle);
	Linker.LinkExternalData(DesiredMovementHandle);
	Linker.LinkExternalData(MovementParamHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FProcessAttack::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(EntityTransformHandle);
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadWrite(AnimationFragmentHandle);
	Builder.AddReadWrite(AttackFragmentHandle);
	Builder.AddReadWrite(DesiredMovementHandle);
	Builder.AddReadWrite(MovementParamHandle);
	Builder.AddReadWrite(MoveTargetHandle);
	Builder.AddReadWrite(MassSignalSubsystemHandle);
}

EStateTreeRunStatus FProcessAttack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FProcessAttack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FTransformFragment& TransformFrag = Context.GetExternalData(EntityTransformHandle);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAttackFragment& AttackFragment = Context.GetExternalData(AttackFragmentHandle);
	FAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationFragmentHandle);
	FMassMovementParameters& MovementParameters = Context.GetExternalData(MovementParamHandle);
	FMassMoveTargetFragment& MoveTargetFragmentFragment = Context.GetExternalData(MoveTargetHandle);
	FMassDesiredMovementFragment& DesiredMovementFragment = Context.GetExternalData(DesiredMovementHandle);

	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);

	AttackFragment.TimeLeftToAttack -= DeltaTime+0.001f;
	
	if (InstanceData.EntityTarget.Index == -1 && InstanceData.EntityTarget.SerialNumber == -1) return EStateTreeRunStatus::Failed;
	FVector TargetLocation = ScaleSubsystem.GetEntityDataLocation(InstanceData.EntityTarget);
	FVector SelfLocation = TransformFrag.GetTransform().GetLocation();
	FVector ForwardDir = (TargetLocation-SelfLocation).GetSafeNormal();
	MoveTargetFragmentFragment.Forward = ForwardDir;
	MoveTargetFragmentFragment.Forward.Z = 0.f;
	float DistanceAway = ((TargetLocation-SelfLocation).Size()-AttackFragment.AttackRange*0.95);
	MoveTargetFragmentFragment.Center = ForwardDir*((TargetLocation-SelfLocation).Size()-DistanceAway);
	MoveTargetFragmentFragment.Center.Z = 0.f;
	DesiredMovementFragment.DesiredVelocity = MoveTargetFragmentFragment.Center.GetSafeNormal()*(FMath::Clamp(DistanceAway*MovementParameters.MaxSpeed-10.f,0,MovementParameters.MaxSpeed));
	DesiredMovementFragment.DesiredFacing = (TargetLocation-SelfLocation).ToOrientationQuat();

	if ((TargetLocation - SelfLocation).Size() >= AttackFragment.AttackRange)
	{
		AnimationFragment.CurrentState = EAnimationState::Running;
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), 0.1f);
		return EStateTreeRunStatus::Running;
	}
	
	if (AttackFragment.TimeLeftToAttack > 0.f)
	{
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), AttackFragment.TimeLeftToAttack+0.01f);
		AnimationFragment.CurrentState = EAnimationState::Idle;
		return EStateTreeRunStatus::Running;
	}

	if (AttackFragment.TimeLeftToAttack <= 0.f && AnimationFragment.CurrentState != EAnimationState::Attacking)
	{
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), AttackFragment.AnimationDelayUntilDamage);
		AnimationFragment.CurrentState = EAnimationState::Attacking;
		AttackFragment.CurrentTimeIntoTheAttack = 0.f;
		return EStateTreeRunStatus::Running;
	}
	if (AttackFragment.CurrentTimeIntoTheAttack < AttackFragment.AnimationDelayUntilDamage)
	{
		AttackFragment.CurrentTimeIntoTheAttack += DeltaTime;
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), AttackFragment.AnimationDelayUntilDamage-AttackFragment.CurrentTimeIntoTheAttack);
		return EStateTreeRunStatus::Running;
	}

	if (AttackFragment.DamageInstances > AttackFragment.CurDamageInstance)
	{
		ScaleSubsystem.DealDamageToEnemy(InstanceData.EntityTarget, AttackFragment.AttackDamage, AttackFragment.DamageType);
		AttackFragment.CurDamageInstance += 1;
	}

	if (AttackFragment.CurrentTimeIntoTheAttack < AttackFragment.AnimationDelayUntilDamage+AttackFragment.AttackRecoveryTime)
	{
		AttackFragment.CurrentTimeIntoTheAttack += DeltaTime;
		MassSignalSubsystem.DelaySignalEntityDeferred(MassStateTreeContext.GetMassEntityExecutionContext(),
		UE::Mass::Signals::StandTaskFinished, MassStateTreeContext.GetEntity(), AttackFragment.AttackRecoveryTime);
		return EStateTreeRunStatus::Running;
	}
	AttackFragment.TimeLeftToAttack = AttackFragment.AttackRate;
	AttackFragment.CurDamageInstance = 0;
	
	AnimationFragment.CurrentState = EAnimationState::Idle;

	
	return EStateTreeRunStatus::Succeeded;
}


FWalkToEntity::FWalkToEntity()
{
	bShouldCallTick = false;
}

bool FWalkToEntity::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(AnimationFragmentHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

void FWalkToEntity::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(EntityTransformHandle);
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadWrite(AnimationFragmentHandle);
	Builder.AddReadWrite(MassSignalSubsystemHandle);
}

EStateTreeRunStatus FWalkToEntity::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationFragmentHandle);

	AnimationFragment.CurrentState = EAnimationState::Running;

	FMassTargetLocation OutLocation = FMassTargetLocation();
	//UE_LOG(LogTemp, Warning, TEXT("Walking to Index: %i , Serial: %i"), InstanceData.EntityTarget.Index, InstanceData.EntityTarget.SerialNumber);
	if (InstanceData.EntityTarget.SerialNumber == -1 && InstanceData.EntityTarget.Index == -1) return EStateTreeRunStatus::Failed;
	const FVector EntityLoc = ScaleSubsystem.GetEntityDataLocation(InstanceData.EntityTarget);
	OutLocation.EndOfPathPosition = EntityLoc; 
	//UE_LOG(LogTemp, Warning, TEXT("Walking to X: %f , Y: %f , Z: %f"), EntityLoc.X, EntityLoc.Y, EntityLoc.Z);
	OutLocation.EndOfPathIntent = EMassMovementAction::Stand;

	InstanceData.LocationTarget = OutLocation;

	return EStateTreeRunStatus::Running;
}




FCheckOwnHealth::FCheckOwnHealth()
{
	bShouldCallTick = true;
}

bool FCheckOwnHealth::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(ScaleSubsystemHandle);
	Linker.LinkExternalData(AnimationFragmentHandle);
	Linker.LinkExternalData(DefenceFragmentHandle);
	Linker.LinkExternalData(DeathPhysicsHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MassCrowdRepHandle);
	Linker.LinkExternalData(MassRepFragmentHandle);
	return true;
}

void FCheckOwnHealth::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadWrite(EntityTransformHandle);
	Builder.AddReadWrite(ScaleSubsystemHandle);
	Builder.AddReadWrite(AnimationFragmentHandle);
	Builder.AddReadWrite(DefenceFragmentHandle);
	Builder.AddReadWrite(DeathPhysicsHandle);
	Builder.AddReadWrite(MassCrowdRepHandle);
	Builder.AddReadWrite(MassRepFragmentHandle);
	Builder.AddReadWrite(MassSignalSubsystemHandle);
}

EStateTreeRunStatus FCheckOwnHealth::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationFragmentHandle);
	FDefenceFragment& DefenceFragment = Context.GetExternalData(DefenceFragmentHandle);

	InstanceData.EntityHealth = DefenceFragment.HP;

	if (DefenceFragment.HP <= 0.f)
	{
		if (AnimationFragment.CurrentState == EAnimationState::Dead) return EStateTreeRunStatus::Running;
		AnimationFragment.CurrentState = EAnimationState::Dead;
		FDeathPhysicsFragment& DeathFragment = Context.GetExternalData(DeathPhysicsHandle);
		FTransformFragment& Transform = Context.GetExternalData(EntityTransformHandle);
		DeathFragment.TotalDeaths += 1;
		DeathFragment.DeathLocations.Add(Transform.GetTransform().GetLocation());
		UMassCrowdRepresentationSubsystem& CrowdRepSubsystem = Context.GetExternalData(MassCrowdRepHandle);
		FMassRepresentationFragment& MassRepresentationFragment = Context.GetExternalData(MassRepFragmentHandle);
		//UE_LOG(LogTemp, Warning, TEXT("Death: %i"), DeathFragment.TotalDeaths);
		if (MassRepresentationFragment.StaticMeshDescHandle.IsValid())
		{
			FMassInstancedStaticMeshInfoArrayView ISMInfosView = CrowdRepSubsystem.GetMutableInstancedStaticMeshInfos();
			FMassInstancedStaticMeshInfo ISMInfo = ISMInfosView[MassRepresentationFragment.StaticMeshDescHandle.ToIndex()];
			Transform.GetMutableTransform().SetScale3D(FVector(0.001f,0.001f,0.001f));
			Transform.GetMutableTransform().SetLocation(FVector(0,0,-9999));
			ScaleSubsystem.NewDeath(ISMInfo.GetDesc().Meshes[0].Mesh->GetName()
				,DeathFragment.TotalDeaths
				,DeathFragment.DeathLocations);
			//UE_LOG(LogTemp, Warning, TEXT("Death Registered: %i"), DeathFragment.TotalDeaths);
		}
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCheckOwnHealth::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassStateTreeContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UScaleSubsystem& ScaleSubsystem = Context.GetExternalData(ScaleSubsystemHandle);
	FAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationFragmentHandle);
	FDefenceFragment& DefenceFragment = Context.GetExternalData(DefenceFragmentHandle);

	InstanceData.EntityHealth = DefenceFragment.HP;

	if (DefenceFragment.HP <= 0.f)
	{
		if (AnimationFragment.CurrentState == EAnimationState::Dead) return EStateTreeRunStatus::Running;
		AnimationFragment.CurrentState = EAnimationState::Dead;
		FDeathPhysicsFragment& DeathFragment = Context.GetExternalData(DeathPhysicsHandle);
		FTransformFragment& Transform = Context.GetExternalData(EntityTransformHandle);
		DeathFragment.TotalDeaths += 1;
		DeathFragment.DeathLocations.Add(Transform.GetTransform().GetLocation());
		UMassCrowdRepresentationSubsystem& CrowdRepSubsystem = Context.GetExternalData(MassCrowdRepHandle);
		FMassRepresentationFragment& MassRepresentationFragment = Context.GetExternalData(MassRepFragmentHandle);
		//UE_LOG(LogTemp, Warning, TEXT("Death: %i"), DeathFragment.TotalDeaths);
		if (MassRepresentationFragment.StaticMeshDescHandle.IsValid())
		{
			FMassInstancedStaticMeshInfoArrayView ISMInfosView = CrowdRepSubsystem.GetMutableInstancedStaticMeshInfos();
			FMassInstancedStaticMeshInfo ISMInfo = ISMInfosView[MassRepresentationFragment.StaticMeshDescHandle.ToIndex()];
			ScaleSubsystem.NewDeath(ISMInfo.GetDesc().Meshes[0].Mesh->GetName()
				,DeathFragment.TotalDeaths
				,DeathFragment.DeathLocations);
			//UE_LOG(LogTemp, Warning, TEXT("Death Registered: %i"), DeathFragment.TotalDeaths);
			Transform.GetMutableTransform().SetScale3D(FVector(0.001f,0.001f,0.001f));
			Transform.GetMutableTransform().SetLocation(FVector(0,0,-9999));
			
			ScaleSubsystem.DestroyEntity(MassStateTreeContext.GetEntity());
		}
	}

	return EStateTreeRunStatus::Running;
}
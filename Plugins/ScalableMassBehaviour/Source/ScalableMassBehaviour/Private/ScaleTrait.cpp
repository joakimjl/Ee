// Fill out your copyright notice in the Description page of Project Settings.

#include "ScaleTrait.h"

#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "ScaleSubsystem.h"


void USmbCarryResourceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	BuildContext.RequireFragment<FAgentRadiusFragment>();

	BuildContext.AddFragment<FResourceFragment>();
}

void USmbAnimTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& MassEntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	
	FAnimationFragment& AnimRef = BuildContext.AddFragment_GetRef<FAnimationFragment>();
	AnimRef = InAnimationFragment.GetValidated();

	const FVertexAnimations VertexFrag = InVertexFrag.GetValidated();
	const FSharedStruct& SharedVertexFrag = MassEntityManager.GetOrCreateSharedFragment(VertexFrag);
	BuildContext.AddSharedFragment(SharedVertexFrag);
}

void USmbDefenceTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& MassEntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	
	FDefenceFragment& DefenceRef = BuildContext.AddFragment_GetRef<FDefenceFragment>();
	DefenceRef = InDefenceFragment.GetValidated();

	const FDeathPhysicsFragment DeathPhysicsFragment = InDeathFragment.GetValidated();
	const FSharedStruct& SharedDeathPhysicsFragment = MassEntityManager.GetOrCreateSharedFragment(DeathPhysicsFragment);
	BuildContext.AddSharedFragment(SharedDeathPhysicsFragment);
}

void USmbStandardAttackTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FAttackFragment& AttackRef = BuildContext.AddFragment_GetRef<FAttackFragment>();
	AttackRef = InAttackFragment.GetValidated();
}

void USmbExistingTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FTransformFragment>();
	FTeamFragment& TeamRef = BuildContext.AddFragment_GetRef<FTeamFragment>();
	TeamRef = InTeamFragment.GetValidated();

	FLocationDataFragment& LocationRef = BuildContext.AddFragment_GetRef<FLocationDataFragment>();
	LocationRef = InDataFragment.GetValidated();
}


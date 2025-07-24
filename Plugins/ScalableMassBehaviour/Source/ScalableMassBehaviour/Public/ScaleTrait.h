// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScalableMassBehaviour.h"
#include "MassEntityTraitBase.h"
#include "MassEntityElementTypes.h"
#include "ScaleFragments.h"

#include "ScaleTrait.generated.h"



UCLASS()
class USmbCarryResourceTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};


UCLASS()
class USmbAnimTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Scale")
	FAnimationFragment InAnimationFragment;

	UPROPERTY(EditAnywhere, Category = "Scale")
	FVertexAnimations InVertexFrag;
};

UCLASS()
class USmbDefenceTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Scale")
	FDefenceFragment InDefenceFragment;

	UPROPERTY(EditAnywhere, Category = "Scale")
	FDeathPhysicsFragment InDeathFragment;
};


UCLASS()
class USmbStandardAttackTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Scale")
	FAttackFragment InAttackFragment;
};

UCLASS()
class USmbExistingTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Scale")
	FTeamFragment InTeamFragment;

	UPROPERTY(EditAnywhere, Category = "Scale")
	FLocationDataFragment InDataFragment;
};



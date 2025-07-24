// Fill out your copyright notice in the Description page of Project Settings.

#include "ScalePhysicsManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "ScaleSubsystem.h"

AScalePhysicsManager::AScalePhysicsManager()
{
	PrimaryActorTick.bCanEverTick = true;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);
	NiagaraComponent->bAutoActivate = false;
}

void AScalePhysicsManager::BeginPlay()
{
	Super::BeginPlay();

	NiagaraComponent->Activate(true);
}

void AScalePhysicsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!RegisteredSelf) {
		bool WorldIsAvail = false;
		auto World = GetWorldChecked(WorldIsAvail);
		if (GetWorld()->HasSubsystem<UScaleSubsystem>()) {
			TObjectPtr<UScaleSubsystem> ScaleSubsystem = GetWorld()->GetSubsystem<UScaleSubsystem>();
			if (StaticMeshParticle)	{
				FString NameOfMesh = StaticMeshParticle->GetName();
				ScaleSubsystem->RegisterPhysicsManager(this,NameOfMesh);
				RegisteredSelf = true;
				NiagaraComponent->SetVariableStaticMesh("UserStaticMesh",StaticMeshParticle);
			}
		}
	}
}

void AScalePhysicsManager::AddPhysicsParticles(TArray<FVector> InVectors, int32 SpawnCount)
{
	FName PositionArray = "SpawnLocations";
	FName Integer = "InTotalSpawned";

	TotalDead = SpawnCount;

	if (NiagaraComponent && NiagaraComponent->IsRegistered())
	{
		NiagaraComponent->SetVariableInt(Integer,SpawnCount);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
			NiagaraComponent,
			PositionArray,
			InVectors
		);
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassArchetypeTypes.h"
#include "SurvivorsFragments.h"
#include "GameFramework/Actor.h"
#include "MSEnemySpawner.generated.h"

USTRUCT()
struct FMSEnemyTypeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FMSEnemyConfigFragment Config;

	UPROPERTY(EditAnywhere)
	FMSVisualFragment Visual;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Count = 2000;
};

UCLASS()
class MASSSURVIVORS_API AMSEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	

	AMSEnemySpawner();

protected:

	virtual void BeginPlay() override;
	void SpawnEnemyGroup(FMassEntityManager& EntityManager, const FMassArchetypeHandle& Archetype, const FMSEnemyTypeDefinition& Type);

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<FMSEnemyTypeDefinition> EnemyTypes;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnRadius = 2000.f;
};

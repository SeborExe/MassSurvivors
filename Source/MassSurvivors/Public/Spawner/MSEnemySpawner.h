// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassArchetypeTypes.h"
#include "SurvivorsFragments.h"
#include "GameFramework/Actor.h"
#include "MSEnemySpawner.generated.h"

UCLASS()
class MASSSURVIVORS_API AMSEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	

	AMSEnemySpawner();

protected:

	virtual void BeginPlay() override;
	void SpawnEnemyGroup(FMassEntityManager& EntityManager, const FMassArchetypeHandle& Archetype, const FMSEnemyConfigFragment& Config, int32 Count);

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FMSEnemyConfigFragment FastEnemyConfig;   // ustaw w edytorze: mało HP, duża szybkość

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FMSEnemyConfigFragment TankEnemyConfig;   // dużo HP, mała szybkość

	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 CountPerType = 2000;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnRadius = 2000.f;
};

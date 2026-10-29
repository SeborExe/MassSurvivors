// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "SurvivorsFragments.h"
#include "MSEnemyDeathObserver.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSEnemyDeathObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:
	UMSEnemyDeathObserver();
	
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
	
	FMassArchetypeHandle OrbArchetype;

	// Dane kulki przygotowane raz, przy inicjalizacji
	UPROPERTY()
	FMSOrbConfigFragment OrbConfig;
	UPROPERTY()
	FMSVisualFragment OrbVisual;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSEnemySeparationProcessor.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSEnemySeparationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSEnemySeparationProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
	TArray<FVector> Positions;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSDebugDrawProcessor.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSDebugDrawProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSDebugDrawProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};

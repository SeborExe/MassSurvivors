// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSLifeTimeProcessor.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSLifeTimeProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSLifeTimeProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};

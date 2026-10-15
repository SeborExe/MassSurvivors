// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSEnemySeekProcessor.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSEnemySeekProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSEnemySeekProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
};

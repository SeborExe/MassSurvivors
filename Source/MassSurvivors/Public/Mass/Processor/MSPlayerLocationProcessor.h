// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSPlayerLocationProcessor.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSPlayerLocationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMSPlayerLocationProcessor();
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override { return false; }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSDamageApplyProcessor.generated.h"

class UMSDamageSubsystem;

UCLASS()
class MASSSURVIVORS_API UMSDamageApplyProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSDamageApplyProcessor();
	
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	FMassEntityQuery EntityQuery;
	
	UPROPERTY()
	TObjectPtr<UMSDamageSubsystem> DamageSubsystem;
};

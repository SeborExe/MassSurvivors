// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSRepresentationProcessor.generated.h"

class UMSRepresentationSubsystem;
class UInstancedStaticMeshComponent;

UCLASS()
class MASSSURVIVORS_API UMSRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSRepresentationProcessor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	TObjectPtr<UMSRepresentationSubsystem> RepresentationSubsystem;

	// Transformy zebrane w tej klatce, osobno dla każdego ISM.
	// Member, żeby tablice zachowywały pamięć między klatkami.
	TMap<TObjectPtr<UInstancedStaticMeshComponent>, TArray<FTransform>> PendingTransforms;
};

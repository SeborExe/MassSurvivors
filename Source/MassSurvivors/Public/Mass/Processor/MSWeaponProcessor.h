// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MSWeaponProcessor.generated.h"

class UMSPlayerSubsystem;
class UMSSpatialGridSubsystem;

UCLASS()
class MASSSURVIVORS_API UMSWeaponProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMSWeaponProcessor();
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override { return false; }
	
protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	UPROPERTY()
	TObjectPtr<UMSPlayerSubsystem> PlayerSubsystem;
	UPROPERTY()
	TObjectPtr<UMSSpatialGridSubsystem> GridSubsystem;
	
	FMassArchetypeHandle ProjectileArchetype;
	float TimeToNextShot = 0.f;
};

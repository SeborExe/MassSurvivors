// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSRepresentationSubsystem.generated.h"

class AMSInstancedMeshActor;

UCLASS()
class MASSSURVIVORS_API UMSRepresentationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	AMSInstancedMeshActor* GetMeshActor() const { return MeshActor; }

private:
	UPROPERTY()
	TObjectPtr<AMSInstancedMeshActor> MeshActor;
};

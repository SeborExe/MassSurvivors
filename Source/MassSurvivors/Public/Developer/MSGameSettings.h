// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurvivorsFragments.h"
#include "Engine/DeveloperSettings.h"
#include "MSGameSettings.generated.h"

class UStaticMesh;
class UMaterialInterface;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Mass Survivors"))
class MASSSURVIVORS_API UMSGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Experience Orb")
	FMSOrbConfigFragment OrbConfig;

	// Soft pointery: ustawienia przechowują ścieżkę do assetu, a nie załadowany obiekt
	UPROPERTY(Config, EditAnywhere, Category = "Experience Orb")
	TSoftObjectPtr<UStaticMesh> OrbMesh;

	UPROPERTY(Config, EditAnywhere, Category = "Experience Orb")
	TSoftObjectPtr<UMaterialInterface> OrbMaterial;

	UPROPERTY(Config, EditAnywhere, Category = "Experience Orb", meta = (ClampMin = "0.01"))
	float OrbMeshScale = 0.3f;
};

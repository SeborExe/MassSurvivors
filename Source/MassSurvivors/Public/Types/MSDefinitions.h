#pragma once

#include "CoreMinimal.h"
#include "SurvivorsFragments.h"
#include "MSDefinitions.generated.h"

USTRUCT()
struct FMSWeaponDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01"))
	float FireInterval = 0.5f; // sekundy między strzałami

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Range = 1500.f;

	UPROPERTY(EditAnywhere)
	FMSProjectileConfigFragment Projectile;

	UPROPERTY(EditAnywhere)
	FMSVisualFragment Visual;
};
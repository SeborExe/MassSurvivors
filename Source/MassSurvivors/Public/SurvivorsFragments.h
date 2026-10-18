// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityElementTypes.h"
#include "SurvivorsFragments.generated.h" 

USTRUCT()
struct FMSEnemyConfigFragment : public FMassConstSharedFragment
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    float MaxHealth = 100.f;
    
    UPROPERTY(EditAnywhere)
    float MovementSpeed = 300.f;
    
    UPROPERTY(EditAnywhere)
    float ContactDamage = 10.f;
    
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01"))
    float AttackInterval = 1.f; // sekundy między atakami
    
    UPROPERTY(EditAnywhere)
    FColor DebugColor = FColor::Red;
    
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0"))
    float CollisionRadius = 50.f;
    
    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<UMaterialInterface> Material;

    UPROPERTY(EditAnywhere, Category = "Visual", meta = (ClampMin = "0.01"))
    float MeshScale = 1.f;
};

USTRUCT()
struct FMSVelocityFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Value = FVector::ZeroVector;
};

USTRUCT()
struct FMSHealthFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float Health = 0.f;
};

USTRUCT()
struct FMSAttackTimerFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float TimeToNextAttack = 0.f;
};

USTRUCT()
struct FMSDeadTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct FMSEnemyTag : public FMassTag
{
    GENERATED_BODY()
};
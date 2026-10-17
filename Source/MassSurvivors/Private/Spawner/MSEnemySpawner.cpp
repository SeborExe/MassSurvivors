// Fill out your copyright notice in the Description page of Project Settings.

#include "Spawner/MSEnemySpawner.h"

#include "MassArchetypeTypes.h"
#include "MassEntitySubsystem.h"
#include "Mass/EntityFragments.h"

AMSEnemySpawner::AMSEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMSEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	const FMassArchetypeHandle EnemyArchetype = EntityManager.CreateArchetype({
		FTransformFragment::StaticStruct(),
		FMSVelocityFragment::StaticStruct(),
		FMSHealthFragment::StaticStruct(),
		FMSAttackTimerFragment::StaticStruct(),
		FMSEnemyTag::StaticStruct(),
		FMSEnemyConfigFragment::StaticStruct()
	});

	// Jeden archetyp, dwa różne configi
	SpawnEnemyGroup(EntityManager, EnemyArchetype, FastEnemyConfig, CountPerType);
	SpawnEnemyGroup(EntityManager, EnemyArchetype, TankEnemyConfig, CountPerType);
}

void AMSEnemySpawner::SpawnEnemyGroup(FMassEntityManager& EntityManager, const FMassArchetypeHandle& Archetype,
	const FMSEnemyConfigFragment& Config, int32 Count)
{
	// Deduplikacja po wartości: dla identycznego configu dostaniemy tę samą instancję
	const FConstSharedStruct& SharedConfig = EntityManager.GetOrCreateConstSharedFragment(Config);

	FMassArchetypeSharedFragmentValues SharedValues;
	SharedValues.Add(SharedConfig);
	SharedValues.Sort(); // Mass wymaga posortowanych wartości

	TArray<FMassEntityHandle> Entities;
	TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
		EntityManager.BatchCreateEntities(Archetype, SharedValues, Count, Entities);

	for (const FMassEntityHandle Entity : Entities)
	{
		// Losowy punkt na okręgu: losowy kąt, a z niego cos/sin
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const FVector Location(FMath::Cos(Angle) * FMath::FRandRange(0.7f, 1.f) * SpawnRadius, FMath::Sin(Angle) * SpawnRadius, 0.f);

		FTransformFragment& TransformFrag = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
		TransformFrag.SetTransform(FTransform(Location));

		FMSHealthFragment& HealthFrag = EntityManager.GetFragmentDataChecked<FMSHealthFragment>(Entity);
		HealthFrag.Health = Config.MaxHealth;
	}
}

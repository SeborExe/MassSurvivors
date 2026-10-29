// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Observer/MSEnemyDeathObserver.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Developer/MSGameSettings.h"
#include "Mass/EntityFragments.h"

UMSEnemyDeathObserver::UMSEnemyDeathObserver() : EntityQuery(*this)
{
	ObservedType = FMSDeadTag::StaticStruct();   // na jaki typ reagujemy
	Operation = EMassObservedOperation::Add;        // na dodanie (jest też Remove)
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
						   | EProcessorExecutionFlags::Server
						   | EProcessorExecutionFlags::Client);
}

void UMSEnemyDeathObserver::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	
	OrbArchetype = EntityManager->CreateArchetype({
	FTransformFragment::StaticStruct(),
	FMSVelocityFragment::StaticStruct(),
	FMSExperienceFragment::StaticStruct(),
	FMSOrbTag::StaticStruct(),
	FMSOrbConfigFragment::StaticStruct(),
	FMSVisualFragment::StaticStruct()
	});

	// Ustawienia z Project Settings. Soft pointery ładujemy raz, tutaj (game thread),
	// żeby observer nie ładował assetów w trakcie gry.
	const UMSGameSettings* Settings = GetDefault<UMSGameSettings>();
	OrbConfig = Settings->OrbConfig;
	OrbVisual.Mesh = Settings->OrbMesh.LoadSynchronous();
	OrbVisual.Material = Settings->OrbMaterial.LoadSynchronous();
	OrbVisual.MeshScale = Settings->OrbMeshScale;
}

void UMSEnemyDeathObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::All);
}

void UMSEnemyDeathObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Dane do kulek ze WSZYSTKICH chunków z tego wywołania
    TArray<FVector> OrbLocations;
    TArray<int32> OrbAmounts;

    EntityQuery.ForEachEntityChunk(Context, [&OrbLocations, &OrbAmounts](FMassExecutionContext& Context)
    {
        // Raz na chunk: cały chunk to jeden typ przeciwnika, więc jedna wartość doświadczenia
        const int32 ExperienceValue = Context.GetConstSharedFragment<FMSEnemyConfigFragment>().ExperienceValue;
        const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            OrbLocations.Add(Transforms[i].GetTransform().GetLocation());
            OrbAmounts.Add(ExperienceValue);
        }

        Context.Defer().DestroyEntities(Context.GetEntities());
    });

    if (OrbLocations.IsEmpty())
    {
        return;
    }

    // JEDNO polecenie dla wszystkich kulek z tej klatki
    Context.Defer().PushCommand<FMassDeferredCreateCommand>(
        [Archetype = OrbArchetype,
         Config = OrbConfig,
         Visual = OrbVisual,
         Locations = MoveTemp(OrbLocations), // przeniesienie, bez kopiowania tablic
         Amounts = MoveTemp(OrbAmounts)](FMassEntityManager& Manager)
        {
            FMassArchetypeSharedFragmentValues SharedValues;
            SharedValues.Add(Manager.GetOrCreateConstSharedFragment(Config));
            SharedValues.Add(Manager.GetOrCreateConstSharedFragment(Visual));
            SharedValues.Sort();

            TArray<FMassEntityHandle> Orbs;
            TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
                Manager.BatchCreateEntities(Archetype, SharedValues, Locations.Num(), Orbs);

            for (int32 i = 0; i < Orbs.Num(); ++i)
            {
                Manager.GetFragmentDataChecked<FTransformFragment>(Orbs[i]).SetTransform(FTransform(Locations[i]));
                Manager.GetFragmentDataChecked<FMSExperienceFragment>(Orbs[i]).Amount = Amounts[i];
                // Prędkość zostaje domyślna (zero): kulka stoi, dopóki nie złapie jej magnes
            }
        });
}

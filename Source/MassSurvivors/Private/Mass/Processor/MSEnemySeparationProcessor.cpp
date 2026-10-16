// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSEnemySeparationProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSEnemySeekProcessor.h"
#include "Types/MSProcessorGroups.h"

// statystyka do pomiaru (thread-safe, działa też na workerach)
DECLARE_STATS_GROUP(TEXT("MassSurvivors"), STATGROUP_MassSurvivors, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Separation"), STAT_MS_Separation, STATGROUP_MassSurvivors);

UMSEnemySeparationProcessor::UMSEnemySeparationProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// W grupie sterowania, ale PO pościgu: Seek ustawia prędkość, separacja ją koryguje
	ExecutionOrder.ExecuteInGroup = MSProcessorGroups::Steering;
	ExecutionOrder.ExecuteAfter.Add(UMSEnemySeekProcessor::StaticClass()->GetFName());
}

void UMSEnemySeparationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMSVelocityFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
}

void UMSEnemySeparationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_MS_Separation);

	// Przejście 1: snapshot pozycji wszystkich przeciwników
	Positions.Reset();
	Positions.Reserve(EntityQuery.GetNumMatchingEntities(EntityManager));
	
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		// TODO (ty): dopisz pozycje wszystkich encji z chunka do Positions
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const FVector EntityPos = Transforms[i].GetTransform().GetLocation();
			Positions.Add(EntityPos);
		}
	});

	constexpr float SeparationStrength = 600.f;
	int32 GlobalIndex = 0;
	
	// Przejście 2: separacja
	EntityQuery.ForEachEntityChunk(Context, [this, &GlobalIndex](FMassExecutionContext& Context)
	{
		const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMSVelocityFragment> Velocities = Context.GetMutableFragmentView<FMSVelocityFragment>();

		// Raz na chunk: promień do kwadratu (patrz niżej, po co)
		const float Radius = Config.CollisionRadius;
		const float RadiusSq = Radius * Radius;

		for (int32 i = 0; i < Context.GetNumEntities(); ++i, ++GlobalIndex)
		{
			const FVector MyPos = Transforms[i].GetTransform().GetLocation();
			FVector Push = FVector::ZeroVector;
			
			for (int32 OtherIndex = 0; OtherIndex < Positions.Num(); ++OtherIndex)
			{
				// (1) Pomijamy siebie po INDEKSIE, a nie po odległości
				if (OtherIndex == GlobalIndex)
				{
					continue;
				}

				// (2) Kierunek OD sąsiada DO mnie = ja − sąsiad
				FVector Offset = MyPos - Positions[OtherIndex];
				Offset.Z = 0.f;

				// (3) Porównanie kwadratów odległości: bez pierwiastka dla dalekich encji
				const float DistSq = Offset.SizeSquared();
				if (DistSq >= RadiusSq || DistSq < KINDA_SMALL_NUMBER)
				{
					continue;
				}

				// (4) Pierwiastek tylko dla prawdziwych sąsiadów
				const float Dist = FMath::Sqrt(DistSq);
				const float Strength = 1.f - Dist / Radius; // 1 przy styku, 0 na krawędzi promienia

				// (5) Offset / Dist = znormalizowany kierunek, bez drugiego pierwiastka
				Push += (Offset / Dist) * Strength;
			}

			FVector& Velocity = Velocities[i].Value;
			Velocity += Push * SeparationStrength;
			Velocity = Velocity.GetClampedToMaxSize(Config.MovementSpeed);
		}
	});
}

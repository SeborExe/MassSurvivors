// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSProjectileHitProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSSpatialGridBuildProcessor.h"
#include "Subsystem/MSDamageSubsystem.h"
#include "Subsystem/MSSpatialGridSubsystem.h"
#include "Types/MSStats.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Damage events"), STAT_MS_DamageEvents, STATGROUP_MassSurvivors);

UMSProjectileHitProcessor::UMSProjectileHitProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
				   | EProcessorExecutionFlags::Server
				   | EProcessorExecutionFlags::Client);
	
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteAfter.Add(UMSSpatialGridBuildProcessor::StaticClass()->GetFName());
}

void UMSProjectileHitProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	
	DamageSubsystem = UWorld::GetSubsystem<UMSDamageSubsystem>(Owner.GetWorld());
}

void UMSProjectileHitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMSProjectileConfigFragment>();
	
	EntityQuery.AddTagRequirement<FMSProjectileTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
	
	EntityQuery.AddSubsystemRequirement<UMSSpatialGridSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMSDamageSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSProjectileHitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (!DamageSubsystem)
	{
		return;
	}

	// Sekwencyjnie: lambda dopisuje do wspólnego bufora zdarzeń (data race w wersji równoległej)
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const UMSSpatialGridSubsystem& Grid = Context.GetSubsystemChecked<UMSSpatialGridSubsystem>();
		UMSDamageSubsystem& Damage = Context.GetMutableSubsystemChecked<UMSDamageSubsystem>();
		const FMSProjectileConfigFragment& Config = Context.GetConstSharedFragment<FMSProjectileConfigFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

		int32 NumHits = 0;

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const FVector ProjectileLocation = Transforms[i].GetTransform().GetLocation();
			const FMassEntityHandle Target = Grid.FindFirstOverlap(ProjectileLocation, Config.Radius);
			if (!Target.IsSet())
			{
				continue;
			}

			// Wykrywamy, ale NIE modyfikujemy przeciwnika: tylko zapisujemy zdarzenie
			Damage.AddEvent(Target, Config.Damage);

			// Pocisk nieprzebijający znika po trafieniu (przy flushu)
			Context.Defer().DestroyEntity(Context.GetEntity(i));
			++NumHits;
		}

		INC_DWORD_STAT_BY(STAT_MS_DamageEvents, NumHits);
	});
}

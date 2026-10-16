// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSEnemySeekProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSPlayerLocationProcessor.h"
#include "Subsystem/MSPlayerSubsystem.h"
#include "Types/MSProcessorGroups.h"

UMSEnemySeekProcessor::UMSEnemySeekProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// Jawna kolejność: najpierw aktualna pozycja gracza, potem kierunek do niej
	ExecutionOrder.ExecuteInGroup = MSProcessorGroups::Steering;
	ExecutionOrder.ExecuteAfter.Add(UMSPlayerLocationProcessor::StaticClass()->GetFName());
}

void UMSEnemySeekProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMSVelocityFragment>(EMassFragmentAccess::ReadWrite);

	// NOWE: wymaganie const shared fragmentu (dostęp zawsze tylko do odczytu)
	EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();

	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);

	// NOWE: wymaganie subsystemu na poziomie query, tylko odczyt
	EntityQuery.AddSubsystemRequirement<UMSPlayerSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMSEnemySeekProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		// Subsystem z kontekstu: Mass wie o tym dostępie dzięki deklaracji w query
		const UMSPlayerSubsystem& PlayerSubsystem = Context.GetSubsystemChecked<UMSPlayerSubsystem>();
		if (!PlayerSubsystem.HasPlayer())
		{
			return;
		}
		const FVector PlayerLocation = PlayerSubsystem.GetPlayerLocation();

		// RAZ NA CHUNK: wszystkie encje w tym chunku mają ten sam config
		const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMSVelocityFragment> Velocities = Context.GetMutableFragmentView<FMSVelocityFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const FVector ToPlayer = PlayerLocation - Transforms[i].GetTransform().GetLocation();
			Velocities[i].Value = ToPlayer.GetSafeNormal2D() * Config.MovementSpeed;
		}
	});
}

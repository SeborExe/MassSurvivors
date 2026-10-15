// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSDebugDrawProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"

UMSDebugDrawProcessor::UMSDebugDrawProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
				   | EProcessorExecutionFlags::Server
				   | EProcessorExecutionFlags::Client);
	
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	bRequiresGameThreadExecution = true;
}

void UMSDebugDrawProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMSHealthFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();
	
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
}

void UMSDebugDrawProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	if (!World) return;
	
	EntityQuery.ForEachEntityChunk(Context, [World](FMassExecutionContext& Context)
	{
		const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMSHealthFragment> Health = Context.GetFragmentView<FMSHealthFragment>();
		
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			const float CurrentHealth = Health[i].Health;
			const float Ratio = FMath::GetMappedRangeValueClamped(FVector2D(0, Config.MaxHealth), FVector2D(0,1), CurrentHealth);
			const FLinearColor BaseColor(Config.DebugColor);
			const FColor Color = (BaseColor * FMath::Lerp(0.25f, 1.f, Ratio)).ToFColor(true);
			
			DrawDebugPoint(World, Transforms[i].GetTransform().GetLocation(), 10.f, Color);
		}
	});
}

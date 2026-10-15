// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSMovementProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSEnemySeekProcessor.h"

UMSMovementProcessor::UMSMovementProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
				   | EProcessorExecutionFlags::Server
				   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	
	ExecutionOrder.ExecuteAfter.Add(UMSEnemySeekProcessor::StaticClass()->GetFName());
}

void UMSMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMSVelocityFragment>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
}

void UMSMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FMSVelocityFragment> Velocities = Context.GetFragmentView<FMSVelocityFragment>();
		
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			FTransform& Transform = Transforms[i].GetMutableTransform();
			const FVector& Velocity = Velocities[i].Value;
			
			FVector Location = Transform.GetLocation() + Velocity * DeltaTime;
			Transform.SetLocation(Location);
		}
	});
}

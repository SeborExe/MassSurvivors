// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Processor/MSLifeTimeProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"

UMSLifeTimeProcessor::UMSLifeTimeProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
			   | EProcessorExecutionFlags::Server
			   | EProcessorExecutionFlags::Client);
	
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMSLifeTimeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Ogólny: wszystko, co ma czas życia, a nie tylko pociski
	EntityQuery.AddRequirement<FMSLifeTimeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
}

void UMSLifeTimeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const TArrayView<FMSLifeTimeFragment> LifeTimes = Context.GetMutableFragmentView<FMSLifeTimeFragment>();

		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			float& RemainingTime = LifeTimes[i].RemainingTime;
			RemainingTime -= DeltaTime;

			if (RemainingTime <= 0.f)
			{
				Context.Defer().DestroyEntity(Context.GetEntity(i));
			}
		}
	});
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Observer/MSEnemyDeathObserver.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"

UMSEnemyDeathObserver::UMSEnemyDeathObserver() : EntityQuery(*this)
{
	ObservedType = FMSDeadTag::StaticStruct();   // na jaki typ reagujemy
	Operation = EMassObservedOperation::Add;        // na dodanie (jest też Remove)
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
						   | EProcessorExecutionFlags::Server
						   | EProcessorExecutionFlags::Client);
}

void UMSEnemyDeathObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
}

void UMSEnemyDeathObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		Context.Defer().DestroyEntities(Context.GetEntities());
	});
}

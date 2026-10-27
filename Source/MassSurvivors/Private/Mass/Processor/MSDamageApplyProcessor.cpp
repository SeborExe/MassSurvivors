// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSDamageApplyProcessor.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/Processor/MSProjectileHitProcessor.h"
#include "Subsystem/MSDamageSubsystem.h"
#include "Types/MSStats.h"

DECLARE_CYCLE_STAT(TEXT("DamageApply"), STAT_MS_DamageApply, STATGROUP_MassSurvivors);

UMSDamageApplyProcessor::UMSDamageApplyProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// Najpierw zdarzenia muszą zostać zebrane
	ExecutionOrder.ExecuteAfter.Add(UMSProjectileHitProcessor::StaticClass()->GetFName());
}

void UMSDamageApplyProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	DamageSubsystem = UWorld::GetSubsystem<UMSDamageSubsystem>(Owner.GetWorld());
}

void UMSDamageApplyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// To query NIE jest iterowane. Służy tylko do zadeklarowania, że procesor zapisuje HP przeciwników
	// (przez dostęp losowy w Execute), żeby Mass nie uruchomił go równolegle z procesorami czytającymi HP.
	EntityQuery.AddRequirement<FMSHealthFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<UMSDamageSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSDamageApplyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_MS_DamageApply);

	if (!DamageSubsystem)
	{
		return;
	}
	
	// Sekwencyjnie i w jednym miejscu: jedyny procesor, który zmienia HP
	for (const FMSDamageEvent& Event : DamageSubsystem->GetEvents())
	{
		// Cel mógł zostać zniszczony, zanim zdarzenie zostało przetworzone.
		// IsEntityValid sprawdza Index i SerialNumber, więc stary handle nie wskaże na inną encję.
		if (!EntityManager.IsEntityValid(Event.Target))
		{
			continue;
		}

		// Dostęp losowy: handle → archetyp → chunk → dane tej konkretnej encji
		const FMassEntityView TargetView(EntityManager, Event.Target);
		FMSHealthFragment* Health = TargetView.GetFragmentDataPtr<FMSHealthFragment>();
		if (!Health)
		{
			continue;
		}

		const float HealthBefore = Health->Health;
		Health->Health -= Event.Damage;

		// Tag tylko przy PRZEJŚCIU przez zero. Dwa trafienia w tej samej klatce
		// (np. dwa pociski) nie wyślą dwóch poleceń dodania taga.
		if (HealthBefore > 0.f && Health->Health <= 0.f)
		{
			Context.Defer().AddTag<FMSDeadTag>(Event.Target);
		}
	}

	// Wszystkie zdarzenia przetworzone: czyścimy bufor (z zachowaniem pamięci)
	DamageSubsystem->ClearEvents();
}

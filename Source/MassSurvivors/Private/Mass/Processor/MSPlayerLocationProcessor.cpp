// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Processor/MSPlayerLocationProcessor.h"

#include "Kismet/GameplayStatics.h"
#include "Subsystem/MSPlayerSubsystem.h"

UMSPlayerLocationProcessor::UMSPlayerLocationProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// Czytamy pawna (UObject), więc tylko game thread.
	// Ten procesor jest malutki (jeden odczyt na klatkę), więc to nic nie kosztuje.
	bRequiresGameThreadExecution = true;
}

void UMSPlayerLocationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Deklaracja dla Mass: ten procesor ZAPISUJE do subsystemu.
	// Procesory, które go czytają, zostaną uporządkowane po nim i nie będą działać równolegle.
	ProcessorRequirements.AddSubsystemRequirement<UMSPlayerSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSPlayerLocationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (!World) return;

	UMSPlayerSubsystem* PlayerSubsystem = World->GetSubsystem<UMSPlayerSubsystem>();
	if (!PlayerSubsystem) return;

	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		PlayerSubsystem->SetPlayerLocation(PlayerPawn->GetActorLocation());
	}
	else
	{
		PlayerSubsystem->ClearPlayer();
	}
}

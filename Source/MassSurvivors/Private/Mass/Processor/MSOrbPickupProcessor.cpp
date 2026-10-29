// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSOrbPickupProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSPlayerLocationProcessor.h"
#include "Subsystem/MSPlayerSubsystem.h"
#include "Types/MSProcessorGroups.h"

UMSOrbPickupProcessor::UMSOrbPickupProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
			   | EProcessorExecutionFlags::Server
			   | EProcessorExecutionFlags::Client);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	
	ExecutionOrder.ExecuteInGroup = MSProcessorGroups::Steering; // ustawia prędkość → przed Movement
	ExecutionOrder.ExecuteAfter.Add(UMSPlayerLocationProcessor::StaticClass()->GetFName());
}

void UMSOrbPickupProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMSVelocityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMSExperienceFragment>(EMassFragmentAccess::ReadOnly);
	
	EntityQuery.AddConstSharedRequirement<FMSOrbConfigFragment>(EMassFragmentPresence::All);
	
	EntityQuery.AddTagRequirement<FMSOrbTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
	
	EntityQuery.AddSubsystemRequirement<UMSPlayerSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSOrbPickupProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Sekwencyjnie: lambda zapisuje do wspólnego licznika doświadczenia w subsystemie gracza
    EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
    {
        UMSPlayerSubsystem& Player = Context.GetMutableSubsystemChecked<UMSPlayerSubsystem>();
        if (!Player.HasPlayer())
        {
            return;
        }

        // Raz na chunk
        const FMSOrbConfigFragment& Config = Context.GetConstSharedFragment<FMSOrbConfigFragment>();
        const float PickupRadiusSq = FMath::Square(Config.PickupRadius);
        const float MagnetRadiusSq = FMath::Square(Config.MagnetRadius);
        const FVector PlayerLocation = Player.GetPlayerLocation();

        const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMSExperienceFragment> Experiences = Context.GetFragmentView<FMSExperienceFragment>();
        const TArrayView<FMSVelocityFragment> Velocities = Context.GetMutableFragmentView<FMSVelocityFragment>();

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            FVector ToPlayer = PlayerLocation - Transforms[i].GetTransform().GetLocation();
            ToPlayer.Z = 0.f; // tylko płaszczyzna XY

            const float DistSq = ToPlayer.SizeSquared();

            if (DistSq <= PickupRadiusSq)
            {
                // Zebrana: doświadczenie do licznika, kulka znika przy flushu
                Player.AddExperience(Experiences[i].Amount);
                Context.Defer().DestroyEntity(Context.GetEntity(i));
                Velocities[i].Value = FVector::ZeroVector;
            }
            else if (DistSq <= MagnetRadiusSq)
            {
                // Magnes: kierunek o długości 1 razy prędkość
                Velocities[i].Value = ToPlayer.GetSafeNormal() * Config.MagnetSpeed;
            }
            else
            {
                Velocities[i].Value = FVector::ZeroVector;
            }
        }
    });
}

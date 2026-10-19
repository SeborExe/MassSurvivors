// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSWeaponProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Kismet/GameplayStatics.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSPlayerLocationProcessor.h"
#include "Mass/Processor/MSSpatialGridBuildProcessor.h"
#include "Subsystem/MSPlayerSubsystem.h"
#include "Player/MSPlayerPawn.h"
#include "Subsystem/MSSpatialGridSubsystem.h"

class UMSPlayerSubsystem;

UMSWeaponProcessor::UMSWeaponProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	bRequiresGameThreadExecution = true;
	
	ExecutionOrder.ExecuteAfter.Add(UMSSpatialGridBuildProcessor::StaticClass()->GetFName());
	ExecutionOrder.ExecuteAfter.Add(UMSPlayerLocationProcessor::StaticClass()->GetFName());
}

void UMSWeaponProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);
	
	PlayerSubsystem = UWorld::GetSubsystem<UMSPlayerSubsystem>(Owner.GetWorld());
	GridSubsystem = UWorld::GetSubsystem<UMSSpatialGridSubsystem>(Owner.GetWorld());
	
	ProjectileArchetype = EntityManager->CreateArchetype({
	FTransformFragment::StaticStruct(),
	FMSVelocityFragment::StaticStruct(),
	FMSLifeTimeFragment::StaticStruct(),
	FMSProjectileTag::StaticStruct(),
	FMSProjectileConfigFragment::StaticStruct(),
	FMSVisualFragment::StaticStruct()
	});
}

void UMSWeaponProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<UMSPlayerSubsystem>(EMassFragmentAccess::ReadOnly);
	ProcessorRequirements.AddSubsystemRequirement<UMSSpatialGridSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMSWeaponProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // 1. Timer: odliczamy zawsze, także gdy nie ma gracza czy celu
    TimeToNextShot -= Context.GetDeltaTimeSeconds();
    if (TimeToNextShot > 0.f)
    {
        return;
    }

    if (!PlayerSubsystem || !GridSubsystem || !PlayerSubsystem->HasPlayer())
    {
        return;
    }

    // 2. Definicja broni z pawna. Bezpieczne, bo procesor działa na game thread,
    //    a nikt poza nim tej definicji nie potrzebuje.
    const AMSPlayerPawn* PlayerPawn = Cast<AMSPlayerPawn>(UGameplayStatics::GetPlayerPawn(EntityManager.GetWorld(), 0));
    if (!PlayerPawn)
    {
        return;
    }
    const FMSWeaponDefinition& Weapon = PlayerPawn->GetWeapon();

    // 3. Cel w zasięgu broni. Brak celu = brak strzału.
    const FVector PlayerLocation = PlayerSubsystem->GetPlayerLocation();
    FVector TargetLocation = FVector::ZeroVector;

    const FMassEntityHandle Target = GridSubsystem->FindNearest(PlayerLocation, TargetLocation, Weapon.Range);
    if (!Target.IsSet())
    {
        return; // timer NIE jest resetowany (pytanie poniżej)
    }
	
	DrawDebugLine(EntityManager.GetWorld(), PlayerLocation, TargetLocation, FColor::Yellow, false, -1.f, 0, 3.f);

    // 4. Strzał: reset timera i polecenie spawnu
    TimeToNextShot = Weapon.FireInterval;

    const FVector Direction = (TargetLocation - PlayerLocation).GetSafeNormal2D();

    Context.Defer().PushCommand<FMassDeferredCreateCommand>(
        [Archetype = ProjectileArchetype,
         Config = Weapon.Projectile,   // kopie: lambda wykona się później, przy flushu
         Visual = Weapon.Visual,
         SpawnLocation = PlayerLocation,
         Direction](FMassEntityManager& Manager)
        {
            FMassArchetypeSharedFragmentValues SharedValues;
            SharedValues.Add(Manager.GetOrCreateConstSharedFragment(Config));
            SharedValues.Add(Manager.GetOrCreateConstSharedFragment(Visual));
            SharedValues.Sort();

            TArray<FMassEntityHandle> Entities;
            TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
                Manager.BatchCreateEntities(Archetype, SharedValues, 1, Entities);

            const FMassEntityHandle Projectile = Entities[0];
            Manager.GetFragmentDataChecked<FTransformFragment>(Projectile).SetTransform(FTransform(Direction.Rotation(), SpawnLocation));
            Manager.GetFragmentDataChecked<FMSVelocityFragment>(Projectile).Value = Direction * Config.Speed;
            Manager.GetFragmentDataChecked<FMSLifeTimeFragment>(Projectile).RemainingTime = Config.LifeTime;
        });
}

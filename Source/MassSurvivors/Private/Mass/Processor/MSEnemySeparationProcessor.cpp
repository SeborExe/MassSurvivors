#include "Mass/Processor/MSEnemySeparationProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSEnemySeekProcessor.h"
#include "Subsystem/MSSpatialGridSubsystem.h"
#include "Types/MSProcessorGroups.h"
#include "Types/MSStats.h"

DECLARE_CYCLE_STAT(TEXT("Separation"), STAT_MS_Separation, STATGROUP_MassSurvivors);
DECLARE_DWORD_COUNTER_STAT(TEXT("Separation pair checks"), STAT_MS_SeparationChecks, STATGROUP_MassSurvivors);

namespace
{
    constexpr float SeparationStrength = 600.f;
    constexpr int32 MaxNeighbors = 12;
}

UMSEnemySeparationProcessor::UMSEnemySeparationProcessor()
    : EntityQuery(*this)
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
                           | EProcessorExecutionFlags::Server
                           | EProcessorExecutionFlags::Client);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;

    // W grupie sterowania, po pościgu: Seek ustawia prędkość, separacja ją koryguje
    ExecutionOrder.ExecuteInGroup = MSProcessorGroups::Steering;
    ExecutionOrder.ExecuteAfter.Add(UMSEnemySeekProcessor::StaticClass()->GetFName());
}

void UMSEnemySeparationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMSVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();
    EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
    EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);

    // Grid tylko czytamy. Zapisuje go GridBuildProcessor, który wykonuje się wcześniej.
    EntityQuery.AddSubsystemRequirement<UMSSpatialGridSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMSEnemySeparationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SCOPE_CYCLE_COUNTER(STAT_MS_Separation);

    // Każdy chunk pisze tylko do prędkości własnych encji, a grid tylko czyta,
    // więc chunki można bezpiecznie przetwarzać równolegle
    EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
    {
        const UMSSpatialGridSubsystem& Grid = Context.GetSubsystemChecked<UMSSpatialGridSubsystem>();
        const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();

        const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMSVelocityFragment> Velocities = Context.GetMutableFragmentView<FMSVelocityFragment>();

        const float Radius = Config.CollisionRadius;
        const float RadiusSq = Radius * Radius;

        // Licznik LOKALNY dla chunka: każdy wątek ma swój, brak data race
        int32 NumChecks = 0;

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            const FMassEntityHandle Self = Context.GetEntity(i);
            const FVector MyPos = Transforms[i].GetTransform().GetLocation();
            const FIntPoint MyCell = Grid.GetCellCoords(MyPos);

            FVector Push = FVector::ZeroVector;
            int32 NumNeighbors = 0;

            // Własna komórka + 8 sąsiednich (rozmiar komórki >= promień kolizji)
            for (int32 DY = -1; DY <= 1 && NumNeighbors < MaxNeighbors; ++DY)
            {
                for (int32 DX = -1; DX <= 1 && NumNeighbors < MaxNeighbors; ++DX)
                {
                    const TArray<FMSGridEntry>* Cell = Grid.FindCell(MyCell + FIntPoint(DX, DY));
                    if (!Cell)
                    {
                        continue;
                    }

                    for (const FMSGridEntry& Other : *Cell)
                    {
                        // Pomijamy siebie po handle'u: działa niezależnie od kolejności chunków
                        if (Other.Entity == Self)
                        {
                            continue;
                        }

                        ++NumChecks;

                        FVector Offset = MyPos - Other.Position; // od sąsiada do mnie
                        Offset.Z = 0.f;

                        const float DistSq = Offset.SizeSquared();
                        if (DistSq >= RadiusSq || DistSq < KINDA_SMALL_NUMBER)
                        {
                            continue;
                        }

                        const float Dist = FMath::Sqrt(DistSq);
                        const float Strength = 1.f - Dist / Radius;
                        Push += (Offset / Dist) * Strength;

                        if (++NumNeighbors >= MaxNeighbors)
                        {
                            break;
                        }
                    }
                }
            }

            FVector& Velocity = Velocities[i].Value;
            Velocity += Push * SeparationStrength;
            Velocity = Velocity.GetClampedToMaxSize(Config.MovementSpeed);
        }

        // Raz na chunk; statystyki są bezpieczne między wątkami
        INC_DWORD_STAT_BY(STAT_MS_SeparationChecks, NumChecks);
    });
}
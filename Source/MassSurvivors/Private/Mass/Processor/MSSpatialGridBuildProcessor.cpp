#include "Mass/Processor/MSSpatialGridBuildProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Mass/EntityFragments.h"
#include "Mass/Processor/MSEnemySeparationProcessor.h"
#include "Subsystem/MSSpatialGridSubsystem.h"
#include "Types/MSStats.h"

DECLARE_CYCLE_STAT(TEXT("GridBuild"), STAT_MS_GridBuild, STATGROUP_MassSurvivors);

UMSSpatialGridBuildProcessor::UMSSpatialGridBuildProcessor()
    : EntityQuery(*this)
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
                           | EProcessorExecutionFlags::Server
                           | EProcessorExecutionFlags::Client);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;

    // Grid musi być gotowy, zanim separacja zacznie z niego czytać
    ExecutionOrder.ExecuteBefore.Add(UMSEnemySeparationProcessor::StaticClass()->GetFName());
}

void UMSSpatialGridBuildProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
    Super::InitializeInternal(Owner, EntityManager);
    
    GridSubsystem = UWorld::GetSubsystem<UMSSpatialGridSubsystem>(Owner.GetWorld());
}

void UMSSpatialGridBuildProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
    EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
    //EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();

    // Ten procesor ZAPISUJE grid
    EntityQuery.AddSubsystemRequirement<UMSSpatialGridSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UMSSpatialGridBuildProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SCOPE_CYCLE_COUNTER(STAT_MS_GridBuild);

    if (!GridSubsystem)
    {
        return;
    }

    UMSSpatialGridSubsystem& Grid = *GridSubsystem;
    Grid.ResetGrid();
    
    /*const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();
    float CollisionRadius = Config.CollisionRadius;*/
    
    EntityQuery.ForEachEntityChunk(Context, [&Grid](FMassExecutionContext& Context)
    {
        const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();

        for (int32 i = 0; i < Context.GetNumEntities(); ++i)
        {
            Grid.Add(Context.GetEntity(i), Transforms[i].GetTransform().GetLocation());
        }
    });
}
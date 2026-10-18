// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Processor/MSRepresentationProcessor.h"

#include "MassExecutionContext.h"
#include "SurvivorsFragments.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Enemy/MSInstancedMeshActor.h"
#include "Mass/EntityFragments.h"
#include "Subsystem/MSRepresentationSubsystem.h"
#include "Types/MSStats.h"

DECLARE_CYCLE_STAT(TEXT("Representation"), STAT_MS_Representation, STATGROUP_MassSurvivors);

namespace
{
	// Dopasowuje liczbę instancji w ISM do liczby transformów i nadpisuje je wszystkie naraz
	void ApplyTransforms(UInstancedStaticMeshComponent& ISM, const TArray<FTransform>& Transforms)
	{
		const int32 CurrentCount = ISM.GetInstanceCount();
		const int32 WantedCount = Transforms.Num();

		if (WantedCount > CurrentCount)
		{
			// Dokładamy brakujące instancje; ich właściwe pozycje ustawi BatchUpdate poniżej
			TArray<FTransform> NewInstances;
			NewInstances.Init(FTransform::Identity, WantedCount - CurrentCount);
			ISM.AddInstances(NewInstances, /*bShouldReturnIndices*/ false, /*bWorldSpace*/ true);
		}
		else if (WantedCount < CurrentCount)
		{
			// Usuwamy nadmiarowe instancje OD KOŃCA, więc nic się nie przesuwa
			TArray<int32> ToRemove;
			ToRemove.Reserve(CurrentCount - WantedCount);
			for (int32 Index = CurrentCount - 1; Index >= WantedCount; --Index)
			{
				ToRemove.Add(Index);
			}
			ISM.RemoveInstances(ToRemove);
		}

		if (WantedCount > 0)
		{
			ISM.BatchUpdateInstancesTransforms(0, Transforms,
				/*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ true, /*bTeleport*/ true);
		}
	}
}

UMSRepresentationProcessor::UMSRepresentationProcessor() : EntityQuery(*this)
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Standalone
					   | EProcessorExecutionFlags::Server
					   | EProcessorExecutionFlags::Client);
	
	ProcessingPhase = EMassProcessingPhase::PostPhysics; // po ruchu
	bRequiresGameThreadExecution = true;                 // ISM to UObject: tylko game thread
}

void UMSRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	// Pobieramy subsystem, a nie actora: actor powstaje dopiero w OnWorldBeginPlay,
	// które może nastąpić po inicjalizacji procesora
	RepresentationSubsystem = UWorld::GetSubsystem<UMSRepresentationSubsystem>(Owner.GetWorld());
}

void UMSRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMSEnemyConfigFragment>();
	EntityQuery.AddTagRequirement<FMSEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMSDeadTag>(EMassFragmentPresence::None);
}

void UMSRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SCOPE_CYCLE_COUNTER(STAT_MS_Representation);

	AMSInstancedMeshActor* MeshActor = RepresentationSubsystem ? RepresentationSubsystem->GetMeshActor() : nullptr;
	if (!MeshActor)
	{
		return;
	}

	// Raz na klatkę: czyścimy tablice, ale zachowujemy ich pamięć
	for (TPair<TObjectPtr<UInstancedStaticMeshComponent>, TArray<FTransform>>& Pair : PendingTransforms)
	{
		Pair.Value.Reset();
	}

	EntityQuery.ForEachEntityChunk(Context, [this, MeshActor](FMassExecutionContext& Context)
	{
		// Raz na chunk: config, a więc mesh i ISM, są wspólne dla całego chunka
		const FMSEnemyConfigFragment& Config = Context.GetConstSharedFragment<FMSEnemyConfigFragment>();
		if (!Config.Mesh)
		{
			return;
		}

		UInstancedStaticMeshComponent* ISM = MeshActor->GetOrCreateISM(Config.Mesh, Config.Material);
		TArray<FTransform>& Transforms = PendingTransforms.FindOrAdd(ISM);

		const TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const FVector Scale(Config.MeshScale);

		Transforms.Reserve(Transforms.Num() + Context.GetNumEntities());
		for (int32 i = 0; i < Context.GetNumEntities(); ++i)
		{
			FTransform Transform = TransformFragments[i].GetTransform();
			Transform.SetScale3D(Scale);
			Transforms.Add(Transform);
		}
	});

	// Raz na klatkę, po zebraniu danych ze wszystkich chunków: jedna aktualizacja na ISM.
	// ISM bez transformów w tej klatce (np. wszyscy przeciwnicy tego typu zginęli)
	// też tu trafia i zostaje wyczyszczony do zera instancji.
	for (TPair<TObjectPtr<UInstancedStaticMeshComponent>, TArray<FTransform>>& Pair : PendingTransforms)
	{
		if (Pair.Key)
		{
			ApplyTransforms(*Pair.Key, Pair.Value);
		}
	}
}

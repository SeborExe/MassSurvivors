// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/MSRepresentationSubsystem.h"

#include "Enemy/MSInstancedMeshActor.h"

void UMSRepresentationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient; // nie zapisuje się z poziomem
	MeshActor = InWorld.SpawnActor<AMSInstancedMeshActor>(SpawnParams);
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSInstancedMeshActor.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class MASSSURVIVORS_API AMSInstancedMeshActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMSInstancedMeshActor();

	// Zwraca ISM dla pary mesh + materiał, tworząc go przy pierwszym użyciu
	UInstancedStaticMeshComponent* GetOrCreateISM(UStaticMesh* Mesh, UMaterialInterface* Material);

private:
	// Komponenty są własnością actora (to on utrzymuje je przy życiu),
	// a mapa służy tylko do szybkiego wyszukania
	TMap<TPair<const UStaticMesh*, const UMaterialInterface*>, TObjectPtr<UInstancedStaticMeshComponent>> ISMs;

};

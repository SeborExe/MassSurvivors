// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/MSInstancedMeshActor.h"

#include "Components/InstancedStaticMeshComponent.h"

AMSInstancedMeshActor::AMSInstancedMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

UInstancedStaticMeshComponent* AMSInstancedMeshActor::GetOrCreateISM(UStaticMesh* Mesh, UMaterialInterface* Material)
{
	const TPair<const UStaticMesh*, const UMaterialInterface*> Key(Mesh, Material);
	if (const TObjectPtr<UInstancedStaticMeshComponent>* Found = ISMs.Find(Key))
	{
		return *Found;
	}

	UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(this);
	ISM->SetupAttachment(RootComponent);
	ISM->SetStaticMesh(Mesh);
	if (Material)
	{
		ISM->SetMaterial(0, Material);
	}

	// Ustawienia pod tysiące poruszających się instancji
	ISM->SetMobility(EComponentMobility::Movable);
	ISM->SetCollisionEnabled(ECollisionEnabled::NoCollision); // kolizje liczymy sami (grid)
	ISM->SetCastShadow(false);                                // cienie dla tysięcy instancji są drogie

	ISM->RegisterComponent();
	ISMs.Add(Key, ISM);
	return ISM;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "MassEntityTypes.h"
#include "MassExternalSubsystemTraits.h"
#include "Subsystems/WorldSubsystem.h"
#include "MSSpatialGridSubsystem.generated.h"

struct FMSGridEntry
{
	FMassEntityHandle Entity; // do pomijania samego siebie, niezależnie od kolejności chunków
	FVector Position;
	float Radius = 0.f;
};

UCLASS()
class MASSSURVIVORS_API UMSSpatialGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// Czyści komórki, ale ZACHOWUJE ich pamięć: od drugiej klatki brak alokacji
	void ResetGrid()
	{
		for (TPair<FIntPoint, TArray<FMSGridEntry>>& Cell : Cells)
		{
			Cell.Value.Reset();
		}
	}
	
	void Add(const FMassEntityHandle Entity, const FVector& Position)
	{
		Cells.FindOrAdd(GetCellCoords(Position)).Add({ Entity, Position });
	}

	const TArray<FMSGridEntry>* FindCell(const FIntPoint& CellCoords) const
	{
		return Cells.Find(CellCoords);
	}

	FIntPoint GetCellCoords(const FVector& Position) const
	{
		// FloorToInt, a NIE rzutowanie na int (patrz niżej)
		return FIntPoint(FMath::FloorToInt(Position.X / CellSize),
						 FMath::FloorToInt(Position.Y / CellSize));
	}
	
	// Zwraca najbliższą encję w promieniu MaxRange albo nieprawidłowy handle, jeśli nikogo nie ma
	FMassEntityHandle FindNearest(const FVector& Position, FVector& OutPosition, float MaxRange) const;
	FMassEntityHandle FindFirstOverlap(const FVector& Position, float Radius) const;
	
private:
	// Musi być >= największego CollisionRadius spośród wszystkich typów przeciwników
	float CellSize = 100.f;

	TMap<FIntPoint, TArray<FMSGridEntry>> Cells;
};

template<>
struct TMassExternalSubsystemTraits<UMSSpatialGridSubsystem> final
{
	enum
	{
		GameThreadOnly = false,
		ThreadSafeWrite = false
	};
};
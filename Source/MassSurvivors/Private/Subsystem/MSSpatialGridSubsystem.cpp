// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MSSpatialGridSubsystem.h"

FMassEntityHandle UMSSpatialGridSubsystem::FindNearest(const FVector& Position, FVector& OutPosition, float MaxRange) const
{
    FMassEntityHandle Best;
    bool bFound = false;

    // Zaczynamy od MaxRange: wszystko dalej odrzucamy od razu, bez osobnego warunku
    float BestDistSq = MaxRange * MaxRange;

    const FIntPoint Center = GetCellCoords(Position);
    const int32 MaxRing = FMath::CeilToInt(MaxRange / CellSize);

    for (int32 Ring = 0; Ring <= MaxRing; ++Ring)
    {
        // Wczesne przerwanie. Pozycja może leżeć w dowolnym miejscu swojej komórki (nawet przy krawędzi),
        // więc najbliższy możliwy punkt pierścienia Ring jest w odległości co najmniej (Ring - 1) * CellSize.
        // Jeśli już to jest dalej niż najlepszy kandydat, dalsze pierścienie nic nie dadzą.
        if (bFound && Ring > 0)
        {
            const float MinRingDist = (Ring - 1) * CellSize;
            if (MinRingDist * MinRingDist > BestDistSq)
            {
                break;
            }
        }

        for (int32 DY = -Ring; DY <= Ring; ++DY)
        {
            for (int32 DX = -Ring; DX <= Ring; ++DX)
            {
                // Tylko brzeg kwadratu, bo wnętrze sprawdziły już poprzednie pierścienie
                if (FMath::Abs(DX) != Ring && FMath::Abs(DY) != Ring)
                {
                    continue;
                }

                const TArray<FMSGridEntry>* Cell = FindCell(Center + FIntPoint(DX, DY));
                if (!Cell)
                {
                    continue;
                }

                for (const FMSGridEntry& Entry : *Cell)
                {
                    const float DistSq = FVector::DistSquared2D(Position, Entry.Position);
                    if (DistSq < BestDistSq)
                    {
                        BestDistSq = DistSq;
                        Best = Entry.Entity;
                        OutPosition = Entry.Position;
                        bFound = true;
                    }
                }
            }
        }
    }

    return Best;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassExternalSubsystemTraits.h"
#include "MSPlayerSubsystem.generated.h"

UCLASS()
class MASSSURVIVORS_API UMSPlayerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	void SetPlayerLocation(const FVector& InLocation)
	{
		PlayerLocation = InLocation;
		bHasPlayer = true;
	}
	
	void ClearPlayer() { bHasPlayer = false; }

	const FVector& GetPlayerLocation() const { return PlayerLocation; }
	bool HasPlayer() const { return bHasPlayer; }
	
	void AddExperience(int32 Amount) { PendingExperience += Amount; }
	int32 ConsumePendingExperience();
	
private:
	// Zwykłe dane, bez wskaźników na UObject: bezpieczne do odczytu z dowolnego wątku,
	// o ile nikt w tym czasie nie zapisuje (tego pilnuje Mass dzięki deklaracjom w procesorach)
	FVector PlayerLocation = FVector::ZeroVector;
	bool bHasPlayer = false;
	
	int32 PendingExperience = 0;
};

// Deklaracja dla Mass: ten subsystem można używać poza game thread.
template<>
struct TMassExternalSubsystemTraits<UMSPlayerSubsystem> final
{
	enum
	{
		GameThreadOnly = false,  // procesory czytające ten subsystem mogą działać na workerach
		ThreadSafeWrite = false  // zapisy nie są thread-safe, więc Mass nie pozwoli na równoległych piszących
	};
};
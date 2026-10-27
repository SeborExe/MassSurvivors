// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassExternalSubsystemTraits.h"
#include "MSDamageSubsystem.generated.h"

struct FMSDamageEvent
{
	FMassEntityHandle Target;
	float Damage = 0.f;
};

UCLASS()
class MASSSURVIVORS_API UMSDamageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void AddEvent(const FMassEntityHandle Target, const float Damage) { Events.Add({ Target, Damage }); }
	TConstArrayView<FMSDamageEvent> GetEvents() const { return Events; }
	void ClearEvents() { Events.Reset(); } // Reset: zachowuje pamięć między klatkami

private:
	TArray<FMSDamageEvent> Events;
};

// Tylko proste dane (handle'e i liczby), więc dostęp poza game thread jest bezpieczny,
// o ile Mass pilnuje kolejności. Zapisy nie są thread-safe.
template<>
struct TMassExternalSubsystemTraits<UMSDamageSubsystem> final
{
	enum
	{
		GameThreadOnly = false,
		ThreadSafeWrite = false
	};
};
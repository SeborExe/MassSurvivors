#pragma once

#include "CoreMinimal.h"

namespace MSProcessorGroups
{
	// Procesory ustalające prędkość (pościg, separacja, później inne zachowania)
	const FName Steering = TEXT("MS.Steering");

	// Procesory przesuwające encje na podstawie prędkości
	const FName Movement = TEXT("MS.Movement");
}
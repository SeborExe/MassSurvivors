// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/MSPlayerSubsystem.h"

int32 UMSPlayerSubsystem::ConsumePendingExperience()
{
	const int32 ExperienceToConsume = PendingExperience;
	PendingExperience = 0;
	return ExperienceToConsume;
}

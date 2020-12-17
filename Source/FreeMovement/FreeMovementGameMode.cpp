// Copyright Epic Games, Inc. All Rights Reserved.

#include "FreeMovementGameMode.h"
#include "FreeMovementCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFreeMovementGameMode::AFreeMovementGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/FreeMovement/BP_FreeMovementCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

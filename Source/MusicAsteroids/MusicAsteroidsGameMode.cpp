// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MusicAsteroidsGameMode.h"
#include "MusicAsteroidsPawn.h"

AMusicAsteroidsGameMode::AMusicAsteroidsGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = AMusicAsteroidsPawn::StaticClass();
}


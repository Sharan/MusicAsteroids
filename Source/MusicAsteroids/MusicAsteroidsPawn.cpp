// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MusicAsteroidsPawn.h"
#include "MusicAsteroidsProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundBase.h"

const FName AMusicAsteroidsPawn::MoveForwardBinding("MoveForward");
const FName AMusicAsteroidsPawn::MoveRightBinding("MoveRight");
//const FName AMusicAsteroidsPawn::FireForwardBinding("FireForward");
//const FName AMusicAsteroidsPawn::FireRightBinding("FireRight");

const FName AMusicAsteroidsPawn::FireWeaponBinding("FireWeapon");

AMusicAsteroidsPawn::AMusicAsteroidsPawn()
{	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;

	//Asteroids
	AsteroidSpawnXOffset = 1000.0f;
	AsteroidSpawnYOffset = 1000.0f;
	InitialNumberOfAsteroids = 5;
	LargeAsteroidsPerLevel = 100;
	MaxAsteroidDistance = 5000.0f;
	AsteroidMoveSpeed = 200.0f;
	SpawnRate = 0.1f;

	//seed random
	double secs = FTimespan(FDateTime::Now().GetTicks()).GetTotalSeconds();
	int32 seed = (int32)(((int64)secs) % INT_MAX);
	FMath::RandInit(seed);
}

void AMusicAsteroidsPawn::BeginPlay()
{
	APawn::BeginPlay();
	UWorld* const World = GetWorld();
	if ((World != NULL))
	{
		SpawnInitialAsteroids();
	}
}

void AMusicAsteroidsPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	//PlayerInputComponent->BindAxis(FireForwardBinding);
	//PlayerInputComponent->BindAxis(FireRightBinding);
	PlayerInputComponent->BindAction(FireWeaponBinding, IE_Pressed, this, &AMusicAsteroidsPawn::HandleFirePressed);
}

void AMusicAsteroidsPawn::Tick(float DeltaSeconds)
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	//const float RightValue = GetInputAxisValue(MoveRightBinding);

	const float RotationDirection = GetInputAxisValue(MoveRightBinding);
	const FVector CurrentDirection = RootComponent->GetForwardVector();

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	//const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = CurrentDirection * MoveSpeed * DeltaSeconds;
	float RotateSpeed = 90.0f;
	if ((RotationDirection * RotationDirection) > 0.0f)
	{
		FRotator AddedRotation = FVector(1, 0, 0).Rotation();
		float TotalRotation = (RotationDirection * RotateSpeed * DeltaSeconds);
		AddedRotation.Add(0, TotalRotation, 0);
		RootComponent->AddLocalRotation(AddedRotation);
	}
	// If non-zero size, move this actor
	if ((ForwardValue * ForwardValue) > 0.0f)
	{
		FRotator NewRotation = Movement.Rotation();

		FHitResult Hit(1.f);
		if (ForwardValue < 0)
		{
			const FVector NewMovement = CurrentDirection * MoveSpeed * DeltaSeconds * ForwardValue;
			RootComponent->MoveComponent(NewMovement, NewRotation, true, &Hit);
		}
		else
		{
			RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);
		}


		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}

	CheckForAsteroidSpawn();
	// Create fire direction vector
	//const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	//const float FireRightValue = GetInputAxisValue(FireRightBinding);
	//const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f);

}

void AMusicAsteroidsPawn::HandleFirePressed()
{
	const FVector FireDirection = RootComponent->GetForwardVector();

	// Try and fire a shot
	FireShot(FireDirection);
}

void AMusicAsteroidsPawn::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				AMusicAsteroidsProjectile* proj = World->SpawnActor<AMusicAsteroidsProjectile>(SpawnLocation, FireRotation);
				proj->SetPlayerPawn(this);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AMusicAsteroidsPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void AMusicAsteroidsPawn::ShotTimerExpired()
{
	bCanFire = true;
}

//Asteroid spawning

void AMusicAsteroidsPawn::SpawnInitialAsteroids()
{
	TArray<FVector> posArray;
	UWorld* const World = GetWorld();
	for (int i = 0; i < InitialNumberOfAsteroids; i++)
	{
		FVector pos = GetAsteroidPositionOffScreen();
		while (posArray.Contains(pos))
		{
			pos = GetAsteroidPositionOffScreen();
		}
		posArray.Push(pos);
		
		if (World != NULL)
		{
			// spawn the asteroid
			FRotator FireRotation = UKismetMathLibrary::FindLookAtRotation(pos, RootComponent->GetComponentLocation());
			//FRotator FireRotation = pos.Rotation();
			AAsteroid* newAsteroid = World->SpawnActor<AAsteroid>(pos, FireRotation);
			Asteroids.Push(newAsteroid);
		}
		
	}
	//bCanSpawn = false;
	if (World != NULL)
	{
		World->GetTimerManager().SetTimer(TimerHandle_AsteroidTimerExpired, this, &AMusicAsteroidsPawn::AsteroidTimerExpired, SpawnRate);
	}
}

void AMusicAsteroidsPawn::AsteroidTimerExpired()
{
	bCanSpawn = true;
}


void AMusicAsteroidsPawn::CheckForAsteroidSpawn()
{
	if (bCanSpawn == true)
	{
		FVector pos = GetAsteroidPositionOffScreen();
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the asteroid
			FRotator FireRotation = UKismetMathLibrary::FindLookAtRotation(pos, RootComponent->GetComponentLocation());
			//FRotator FireRotation = pos.Rotation();
			AAsteroid* newAsteroid = World->SpawnActor<AAsteroid>(pos, FireRotation);
			Asteroids.Push(newAsteroid);
		}
		bCanSpawn = false;
	}
}

FVector AMusicAsteroidsPawn::GetAsteroidPositionOffScreen()
{
	FVector NewVector = RootComponent->GetComponentLocation();

	while (NewVector == RootComponent->GetComponentLocation())
	{
		int32 xLoc = FMath::Rand() % 3;
		float xVal;
		switch (xLoc) {
		case 0:
			xVal = RootComponent->GetComponentLocation().X - AsteroidSpawnXOffset;
			break;
		case 1:
			xVal = RootComponent->GetComponentLocation().X;
			break;
		case 2:
			xVal = RootComponent->GetComponentLocation().X + AsteroidSpawnXOffset;
			break;
		}
		NewVector.X = xVal;

		int32 yLoc = FMath::Rand() % 3;
		float yVal;
		switch (yLoc) {
		case 0:
			yVal = RootComponent->GetComponentLocation().Y - AsteroidSpawnYOffset;
			break;
		case 1:
			yVal = RootComponent->GetComponentLocation().Y;
			break;
		case 2:
			yVal = RootComponent->GetComponentLocation().Y + AsteroidSpawnYOffset;
			break;
		}
		NewVector.Y = yVal;
	}

	return NewVector;
}

void AMusicAsteroidsPawn::LoseLife()
{

}

void AMusicAsteroidsPawn::DestroyAsteroid(AAsteroid* asteroid)
{
	int32 idx = Asteroids.Find(asteroid);
	if (idx != INDEX_NONE)
	{
		//TODO: Spawn smaller asteroids until small size
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			asteroid->Destroy();
			World->RemoveActor(asteroid, true);
		}
		Asteroids.Remove(asteroid);
	}
}
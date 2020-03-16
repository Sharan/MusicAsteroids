// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Asteroid.h"
#include "Containers/Array.h"
#include "GameFramework/Character.h"
#include "MusicAsteroidsPawn.generated.h"

UCLASS(Blueprintable)
class AMusicAsteroidsPawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	AMusicAsteroidsPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;
	
	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	/* Offset position offscreen to spawn asteroids */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float AsteroidSpawnXOffset;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float AsteroidSpawnYOffset;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int32 InitialNumberOfAsteroids;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int32 LargeAsteroidsPerLevel;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MaxAsteroidDistance;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float AsteroidMoveSpeed;

	/* How fast the asteroids will spawn */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float SpawnRate;

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;
	// End Actor Interface

	/* Fire a shot in the specified direction */
	void FireShot(FVector FireDirection);
	void HandleFirePressed();

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	/* Handle Asteroid spawning*/
	void SpawnInitialAsteroids();
	void CheckForAsteroidSpawn();
	FVector GetAsteroidPositionOffScreen();
	void AsteroidTimerExpired();

	void LoseLife();
	void DestroyAsteroid(AAsteroid* asteroid);

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	//static const FName FireForwardBinding;
	//static const FName FireRightBinding;

	static const FName FireWeaponBinding;


private:

	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	TArray<AAsteroid*> Asteroids;

	/* Flag to control firing  */
	uint32 bCanSpawn : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_AsteroidTimerExpired;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};


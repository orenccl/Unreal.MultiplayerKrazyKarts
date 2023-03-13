// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	void SimulateMove(const FGoKartMove &Move);

	FVector GetVelocity() { return Velocity; };
	void SetVelocity(FVector Val) { Velocity = Val; };

	void SetThrottle(float Val) { Throttle = Val; };
	void SetSteeringThrow(float Val) { SteeringThrow = Val; };

	FGoKartMove GetLastMove() { return LastMove; };

private:
	APawn *Owner;

	// The mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// The force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// Minimum radius of the car turning circlr at full lock (m)
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	// Higher means more drag (kg/m) AirResistance = -Speed^2 * DragCoefficient. 10000 / 25^2 = 16
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 6.25;

	/**
	 * Higher means more rolling resistance. RollingResistance = -NormalForce * RollingCoefficient. NormalForce = m*g
	 * RollingCoefficient value could refer to wiki
	 */
	UPROPERTY(EditAnywhere)
	float RollingCoefficient = 0.015;

	float Throttle;

	float SteeringThrow;

	FVector Velocity;

	FGoKartMove LastMove;

	FGoKartMove CreateMove(float DeltaTime);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float InSteeringThrow, float DeltaTime);
};

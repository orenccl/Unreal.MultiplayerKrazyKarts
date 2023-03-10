// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	// Moves not acknowlege by server yet
	TArray<FGoKartMove> UnacknowlegedMoves;

	UFUNCTION(Server, Reliable, WithValidation) // ServerRPC
	void Server_SendMove(FGoKartMove Move);

	UFUNCTION()
	void OnRep_ReplicatedServerState();

	void ClearAcknowlegedMoves(FGoKartMove LastMove);

	// Move forward and backword
	void MoveForward(float AxisValue);
	// Move right and left
	void MoveRight(float AxisValue);

	UPROPERTY(EditAnywhere)
	UGoKartMovementComponent* MovementComponent;
	//
	//
	//

	// void SimulateMove(const FGoKartMove &Move);

	// FGoKartMove CreateMove(float DeltaTime);

	// FVector GetAirResistance();

	// FVector GetRollingResistance();

	// void UpdateLocationFromVelocity(float DeltaTime);

	// void ApplyRotation(float InSteeringThrow, float DeltaTime);

	// // The mass of the car (kg)
	// UPROPERTY(EditAnywhere)
	// float Mass = 1000;

	// // The force applied to the car when the throttle is fully down (N)
	// UPROPERTY(EditAnywhere)
	// float MaxDrivingForce = 10000;

	// // Minimum radius of the car turning circlr at full lock (m)
	// UPROPERTY(EditAnywhere)
	// float MinTurningRadius = 10;

	// // Higher means more drag (kg/m) AirResistance = -Speed^2 * DragCoefficient. 10000 / 25^2 = 16
	// UPROPERTY(EditAnywhere)
	// float DragCoefficient = 6.25;

	// /**
	//  * Higher means more rolling resistance. RollingResistance = -NormalForce * RollingCoefficient. NormalForce = m*g
	//  * RollingCoefficient value could refer to wiki
	//  */
	// UPROPERTY(EditAnywhere)
	// float RollingCoefficient = 0.015;

	// float Throttle;

	// float SteeringThrow;

	// FVector Velocity;
};

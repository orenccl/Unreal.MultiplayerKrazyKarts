// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicatior.generated.h"

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

/**
 * Slope = Derivative = Tangent = DeltaLocation / DeltaAlpha
 * Velocity = DeltaLocation / DeltaTime
 * DeltaAlpha = DeltaTime / ClientTimeBetweenLastUpdate
 *
 * Derivative = Velocity * ClientTimeBetweenLastUpdate;
 */
struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FVector InterplateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FVector InterplateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementReplicatior : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementReplicatior();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	APawn *Owner;
	UGoKartMovementComponent *MovementComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	// Moves not acknowlege by server yet
	TArray<FGoKartMove> UnacknowlegedMoves;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdate;

	FTransform ClinetStartTransform;
	FVector ClinetStartVelocity;

	UFUNCTION(Server, Reliable, WithValidation) // ServerRPC
	void Server_SendMove(FGoKartMove Move);

	UFUNCTION()
	void OnRep_ReplicatedServerState();
	void AutonomousProxy_OnRep_ReplicatedServerState();
	void SimulatedProxy_OnRep_ReplicatedServerState();

	void ClearAcknowlegedMoves(FGoKartMove LastMove);

	// Update server state to clients
	void UpdateServerState(const FGoKartMove &Move);

	void ClientTick(float DeltaTime);

	FHermiteCubicSpline CreateSpline();
	float VelocityToDerivative();
	void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);
};

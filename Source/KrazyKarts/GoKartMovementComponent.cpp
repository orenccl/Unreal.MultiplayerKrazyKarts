// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove &Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	// F = ma
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * Move.DeltaTime;

	UpdateLocationFromVelocity(Move.DeltaTime);
	ApplyRotation(Move.SteeringThrow, Move.DeltaTime);
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	// Moving translation (Velocity * 100 convert from m to cm, unreal world use cm for unit)
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult OutSweepHitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &OutSweepHitResult);

	// Clear velocity if hit object
	if (OutSweepHitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void UGoKartMovementComponent::ApplyRotation(float InSteeringThrow, float DeltaTime)
{
	// dX
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	// dCita = dX / r
	float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);
	GetOwner()->AddActorWorldRotation(RotationDelta, true);
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	// Convert to meter
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	// F = m*g
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * NormalForce * RollingCoefficient;
}
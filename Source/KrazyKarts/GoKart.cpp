// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Engine/World.h"

// Sets default values
AGoKart::AGoKart()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	// F = ma
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), UEnum::GetValueAsString(GetLocalRole()), this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Bind event
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float AxisValue)
{
	Throttle = AxisValue;
	Server_MoveForward(AxisValue);
}

// Only get called on server.
void AGoKart::Server_MoveForward_Implementation(float AxisValue)
{
	Throttle = AxisValue;
}

// Only get called on server, check if it cheat
bool AGoKart::Server_MoveForward_Validate(float AxisValue)
{
	return FMath::Abs(AxisValue) <= 1;
}

void AGoKart::MoveRight(float AxisValue)
{
	SteeringThrow = AxisValue;
	Server_MoveRight(AxisValue);
}

void AGoKart::Server_MoveRight_Implementation(float AxisValue)
{
	SteeringThrow = AxisValue;
}

bool AGoKart::Server_MoveRight_Validate(float AxisValue)
{
	return FMath::Abs(AxisValue) <= 1;
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	// Moving translation (Velocity * 100 convert from m to cm, unreal world use cm for unit)
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult OutSweepHitResult;
	AddActorWorldOffset(Translation, true, &OutSweepHitResult);

	// Clear velocity if hit object
	if (OutSweepHitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	// dX
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	// dCita = dX / r
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta, true);
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	// Convert to meter
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	// F = m*g
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * NormalForce * RollingCoefficient;
}

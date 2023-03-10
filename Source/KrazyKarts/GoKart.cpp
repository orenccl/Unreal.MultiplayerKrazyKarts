// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
AGoKart::AGoKart()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	// Server net update frequency 1 per second.
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Bind Replicate
	DOREPLIFETIME(AGoKart, ServerState);
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// We are the client and in control of the pawn
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		// Simultaneous simulate move(Will run ahead of the server)
		SimulateMove(Move);
		// Record move
		UnacknowlegedMoves.Add(Move);
		// Send move information to server
		Server_SendMove(Move);
	}
	// We are the server and in control of the pawn
	else if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}
	// We are the client and control by server
	else if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}

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

// Only get called on server.
void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);
	// Update server state to clients
	ServerState.LastMove = Move;
	ServerState.Velocity = Velocity;
	ServerState.Transform = GetActorTransform();
}

// Only get called on server, check if it cheat
bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return FMath::Abs(Move.Throttle) <= 1 && FMath::Abs(Move.SteeringThrow) <= 1;
}

void AGoKart::OnRep_ReplicatedServerState()
{
	// Sync with server
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowlegedMoves(ServerState.LastMove);

	// Then replay move, still keep ahead of server
	for (const auto &Move : UnacknowlegedMoves)
	{
		SimulateMove(Move);
	}
}

void AGoKart::SimulateMove(const FGoKartMove &Move)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	// F = ma
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * Move.DeltaTime;

	UpdateLocationFromVelocity(Move.DeltaTime);
	ApplyRotation(Move.SteeringThrow, Move.DeltaTime);
}

FGoKartMove AGoKart::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
}

void AGoKart::ClearAcknowlegedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const auto &Move : UnacknowlegedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowlegedMoves = NewMoves;
}

void AGoKart::MoveForward(float AxisValue)
{
	Throttle = AxisValue;
}

void AGoKart::MoveRight(float AxisValue)
{
	SteeringThrow = AxisValue;
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

void AGoKart::ApplyRotation(float InSteeringThrow, float DeltaTime)
{
	// dX
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	// dCita = dX / r
	float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
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

// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicatior.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UGoKartMovementReplicatior::UGoKartMovementReplicatior()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}

// Called when the game starts
void UGoKartMovementReplicatior::BeginPlay()
{
	Super::BeginPlay();
	Owner = Cast<APawn>(GetOwner());
	MovementComponent = Owner->FindComponentByClass<UGoKartMovementComponent>();
}

// Called every frame
void UGoKartMovementReplicatior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr || Owner == nullptr)
		return;

	// We are the client and in control of the pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		// Simultaneous simulate move(Will run ahead of the server)
		MovementComponent->SimulateMove(Move);
		// Record move
		UnacknowlegedMoves.Add(Move);
		// Send move information to server
		Server_SendMove(Move);
	}
	// We are the server and in control of the pawn
	else if (GetOwnerRole() == ROLE_Authority && Owner->IsLocallyControlled())
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}
	// We are the client and control by server
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}

void UGoKartMovementReplicatior::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Bind Replicate
	DOREPLIFETIME(UGoKartMovementReplicatior, ServerState);
}

// Only get called on server.
void UGoKartMovementReplicatior::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr || Owner == nullptr)
		return;

	MovementComponent->SimulateMove(Move);
	// Update server state to clients
	ServerState.LastMove = Move;
	ServerState.Velocity = MovementComponent->GetVelocity();
	ServerState.Transform = Owner->GetActorTransform();
}

// Only get called on server, check if it cheat
bool UGoKartMovementReplicatior::Server_SendMove_Validate(FGoKartMove Move)
{
	return FMath::Abs(Move.Throttle) <= 1 && FMath::Abs(Move.SteeringThrow) <= 1;
}

void UGoKartMovementReplicatior::OnRep_ReplicatedServerState()
{
	if (MovementComponent == nullptr || Owner == nullptr)
		return;

	// Sync with server
	Owner->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowlegedMoves(ServerState.LastMove);

	// Then replay move, still keep ahead of server
	for (const auto &Move : UnacknowlegedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicatior::ClearAcknowlegedMoves(FGoKartMove LastMove)
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
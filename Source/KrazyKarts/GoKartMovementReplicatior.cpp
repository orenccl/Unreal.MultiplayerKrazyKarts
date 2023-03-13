// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicatior.h"
#include "Net/UnrealNetwork.h"

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
	if (Owner == nullptr)
		return;

	MovementComponent = Owner->FindComponentByClass<UGoKartMovementComponent>();
}

// Called every frame
void UGoKartMovementReplicatior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr || Owner == nullptr)
		return;

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	// We are the client and in control of the pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		// Record move
		UnacknowlegedMoves.Add(LastMove);
		// Send move information to server
		Server_SendMove(LastMove);
	}
	// We are the server and in control of the pawn
	else if (GetOwnerRole() == ROLE_Authority && Owner->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}
	// We are the client and control by server
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UGoKartMovementReplicatior::UpdateServerState(const FGoKartMove &Move)
{
	if (Owner == nullptr)
		return;

	// Update server state to clients
	ServerState.LastMove = Move;
	ServerState.Velocity = MovementComponent->GetVelocity();
	ServerState.Transform = Owner->GetActorTransform();
}

void UGoKartMovementReplicatior::ClientTick(float DeltaTime)
{
	if (Owner == nullptr)
		return;

	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER)
	{
		return;
	}

	FVector StartLocation = ClinetStartTransform.GetLocation();
	FVector TargetLocation = ServerState.Transform.GetLocation();
	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

	FVector NewLocation = FMath::LerpStable(StartLocation, TargetLocation, LerpRatio);
	Owner->SetActorLocation(NewLocation);

	FQuat StartRotation = ClinetStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	Owner->SetActorRotation(NewRotation);
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
	if (MovementComponent == nullptr)
		return;

	MovementComponent->SimulateMove(Move);
	UpdateServerState(Move);
}

// Only get called on server, check if it cheat
bool UGoKartMovementReplicatior::Server_SendMove_Validate(FGoKartMove Move)
{
	return FMath::Abs(Move.Throttle) <= 1 && FMath::Abs(Move.SteeringThrow) <= 1;
}

void UGoKartMovementReplicatior::OnRep_ReplicatedServerState()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		AutonomousProxy_OnRep_ReplicatedServerState();
	}
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		SimulatedProxy_OnRep_ReplicatedServerState();
	}
}

void UGoKartMovementReplicatior::AutonomousProxy_OnRep_ReplicatedServerState()
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

void UGoKartMovementReplicatior::SimulatedProxy_OnRep_ReplicatedServerState()
{
	if (Owner == nullptr)
		return;

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;

	ClinetStartTransform = Owner->GetTransform();
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

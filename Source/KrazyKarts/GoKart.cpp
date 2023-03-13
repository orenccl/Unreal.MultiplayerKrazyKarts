// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

// Sets default values
AGoKart::AGoKart()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
	MovementReplicatior = CreateDefaultSubobject<UGoKartMovementReplicatior>(TEXT("MovementReplicatior"));
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(false);

	// Server net update frequency 1 per second.
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: Remove debug code
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
	if (MovementComponent == nullptr)
		return;

	MovementComponent->SetThrottle(AxisValue);
}

void AGoKart::MoveRight(float AxisValue)
{
	if (MovementComponent == nullptr)
		return;

	MovementComponent->SetSteeringThrow(AxisValue);
}

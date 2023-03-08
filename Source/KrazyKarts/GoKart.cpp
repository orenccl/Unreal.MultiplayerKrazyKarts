// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

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
	// Moving
	FVector Translation = Velocity * DeltaTime;
	AddActorWorldOffset(Translation, true);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Bind event
	PlayerInputComponent->BindAxis("MoveFword", this, &AGoKart::MoveFword);
}

void AGoKart::MoveFword(float AxisValue)
{
	// 20 meter * 100 centimeter(world use centimeter for a unit)
	Velocity = GetActorForwardVector() * 20 * 100 * AxisValue;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthCharacter.h"
#include "PlayerEnergyComponent.h"

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}	

void AStealthCharacter::DashAbility()
{
	Dash();
}


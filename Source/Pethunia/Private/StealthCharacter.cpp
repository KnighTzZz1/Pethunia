// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthCharacter.h"
#include "PlayerEnergyComponent.h"
#include "TimerManager.h"

AStealthCharacter::AStealthCharacter()
{
	IncreasedEnergyMultiplier = 1.0f;
	energyComp = EnergyComponent;
	PowerDuration = 3.0f;
	PowerCooldown = 5.0f;
	PowerCost = 40.f;
}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}	

void AStealthCharacter::DashAbility()
{
	Dash();
	
}

void AStealthCharacter::Power1Activate()
{
	
	if (bPower1IsOnCooldown == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Power Activated"));
		if (EnergyComponent && EnergyComponent->CurrentEnergy - PowerCost >= 0)
		{
			EnergyComponent->UseEnergy(PowerCost);

			SprintSpeedMultiplier *= 1.2f;
			JumpHeight *= 1.5;
			bPower1IsOnCooldown = true;
			GetWorldTimerManager().SetTimer(Power1Handle, this, &AStealthCharacter::Power1Deactivate, PowerDuration, false);
			
		}
	}
}

void AStealthCharacter::Power1Deactivate()
{
	UE_LOG(LogTemp, Error, TEXT("Power Deactivated"));
	if (EnergyComponent) {
		GetWorldTimerManager().SetTimer(Power1CooldownHandle, this, &AStealthCharacter::PowerCooldownOff, PowerCooldown, false);
		EnergyComponent->DashCost /= IncreasedEnergyMultiplier;
		SprintSpeedMultiplier /= 1.2f;
		JumpHeight /= 1.5;
	}
}

void AStealthCharacter::PowerCooldownOff()
{
	bPower1IsOnCooldown = false;
}
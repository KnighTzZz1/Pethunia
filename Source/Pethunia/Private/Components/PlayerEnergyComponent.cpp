// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerEnergyComponent.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UPlayerEnergyComponent::UPlayerEnergyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	regEnergy = false;
	Energy = MaxEnergy;
	RegAmmount = 0.1f;
}


// Called when the game starts
void UPlayerEnergyComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UPlayerEnergyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Energy < MaxEnergy && regEnergy)
	{
		Energy += RegAmmount;
		if (Energy == MaxEnergy) regEnergy = false;
	}
}

void UPlayerEnergyComponent::UseEnergy(float value)
{
	if (Energy - value >= 0)
	{
		regEnergy = false;
		Energy -= value;
		GetWorld()->GetTimerManager().SetTimer(EnergyRechargeTimer, this, &UPlayerEnergyComponent::regenerateEnergyFunction, 2.0f, false);
	}
}

void UPlayerEnergyComponent::regenerateEnergyFunction()
{
	regEnergy = true;
}

void UPlayerEnergyComponent::GetEnergy(float value)
{
	if (Energy < MaxEnergy)
	{
		Energy += value;
	}
	if (Energy >= MaxEnergy) Energy = MaxEnergy;
}
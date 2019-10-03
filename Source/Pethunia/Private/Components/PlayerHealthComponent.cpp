// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthComponent.h"

// Sets default values for this component's properties
UPlayerHealthComponent::UPlayerHealthComponent()
{
	Health = 100.f;
}


// Called when the game starts
void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();

}



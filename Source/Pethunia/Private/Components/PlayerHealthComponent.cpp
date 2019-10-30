// Fill out your copyright notice in the Description page of Project Settings.	


#include "PlayerHealthComponent.h"


// Sets default values for this component's properties
UPlayerHealthComponent::UPlayerHealthComponent()
{
	MaxHealth = 100.f;
	Health = MaxHealth;
}


// Called when the game starts
void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UPlayerHealthComponent::TakeDamage(float damage)
{
	Health -= damage;
	if(Health <= 0)
	{
		GetOwner()->Destroy();
	}
}


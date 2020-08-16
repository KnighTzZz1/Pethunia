// Fill out your copyright notice in the Description page of Project Settings.	


#include "PlayerHealthComponent.h"
#include "UnrealNetwork.h"
#include "PlayerCharacter.h"

// Sets default values for this component's properties
UPlayerHealthComponent::UPlayerHealthComponent()
{
	SetIsReplicated(true);

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
		if (GetOwner()->ActorHasTag(FName(TEXT("Player"))))
		{
			APlayerCharacter* Player = (APlayerCharacter*)GetOwner();
			Player->Die();
		}
		else
		{
			GetOwner()->Destroy();
		}
	}
}

void UPlayerHealthComponent::_Experimental_SetHealth(float value)
{
	Health = value;
}


void UPlayerHealthComponent::GetLifetimeReplicatedProps(TArray < FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPlayerHealthComponent, Health);
}

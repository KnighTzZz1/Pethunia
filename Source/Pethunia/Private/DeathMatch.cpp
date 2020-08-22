// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathMatch.h"
#include "Net/UnrealNetwork.h"
#include "PethuniaPlayerState.h"

ADeathMatch::ADeathMatch()
{
	
}

void ADeathMatch::BeginPlay()
{

}

void ADeathMatch::PostLogin(APlayerController* Controller)
{
	Controllers.Add(Controller);
}

void ADeathMatch::AssignColors()
{
	for (APlayerController* c : Controllers)
	{
		APethuniaPlayerState* state = (APethuniaPlayerState*)c->PlayerState;
		state->SetPlayerColor(FColor::MakeRandomColor());
	}
}

void ADeathMatch::OnPlayerDeath()
{
	for (APlayerController* c : Controllers)
	{
		c->ServerRestartPlayer();
	}
}

void ADeathMatch::AddStartPosition(APlayerStart* playerStart)
{
	StartPositions.Add(playerStart);
}

void ADeathMatch::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatch, Controllers);
	DOREPLIFETIME(ADeathMatch, StartPositions);
}


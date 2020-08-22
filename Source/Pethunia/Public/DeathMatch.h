// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerStart.h"
#include "DeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class PETHUNIA_API ADeathMatch : public AGameMode
{
	GENERATED_BODY()

public:
	ADeathMatch();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* Controller) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
		void AssignColors();

	UFUNCTION()
		void OnPlayerDeath();

	UFUNCTION()
		void AddStartPosition(APlayerStart* playerStart);


	UPROPERTY(Replicated, BlueprintReadWrite)
		TArray<APlayerController*> Controllers;

	UPROPERTY(Replicated, BlueprintReadOnly)
		TArray<APlayerStart*> StartPositions;

};

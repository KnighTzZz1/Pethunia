// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PethuniaPlayerState.generated.h"


/**
 * 
 */
UCLASS()
class PETHUNIA_API APethuniaPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	FColor PlayerColor;

	UFUNCTION()
		void SetPlayerColor(FColor color);
};

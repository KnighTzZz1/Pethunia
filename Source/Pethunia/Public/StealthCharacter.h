// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "StealthCharacter.generated.h"

class UCameraComponent;

/**
 * 
 */
UCLASS()
class PETHUNIA_API AStealthCharacter : public APlayerCharacter
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime);
	

	virtual void DashAbility() override;
	UFUNCTION(BlueprintImplementableEvent)
		void Dash();

private:
	
};

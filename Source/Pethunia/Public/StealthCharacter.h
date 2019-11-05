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
	AStealthCharacter();
	virtual void Tick(float DeltaTime);
	

	virtual void DashAbility() override;
	UFUNCTION(BlueprintImplementableEvent)
		void Dash();

	
	virtual void Power1Activate() override;
	virtual void Power1Deactivate() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "1.0", ClampMax = "2.0"))
		float IncreasedEnergyMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "1.0",ClampMax = "10.0"))
		float PowerDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0",ClampMax = "20"))
		float PowerCooldown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
		float PowerCost;
	
private:
	AActor* Parent;
	UPlayerEnergyComponent* energyComp;
	FTimerHandle Power1Handle;
	FTimerHandle Power1CooldownHandle;
	bool bPower1IsOnCooldown;
	void PowerCooldownOff();

};

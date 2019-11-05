// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerEnergyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PETHUNIA_API UPlayerEnergyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerEnergyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float RegAmmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0"))
		float DashCost;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		float CurrentEnergy;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
		float MaxEnergy = 100.f;
	UFUNCTION(BlueprintCallable)
		void UseEnergy(float value);
	UFUNCTION(BlueprintCallable)
		void GetEnergy(float value);

private:
	bool test;
	bool regEnergy;
	void regenerateEnergyFunction();
	FTimerHandle EnergyRechargeTimer;
};

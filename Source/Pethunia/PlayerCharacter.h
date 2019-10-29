// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UPlayerHealthComponent;
class UPlayerEnergyComponent;

UCLASS()
class PETHUNIA_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	APlayerCharacter();
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UPlayerHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UPlayerEnergyComponent* EnergyComponent;

	void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	FTimerHandle StaminaRechargeTimer;
protected:
	void BeginPlay() override;

	// Movement
	void MoveForward(float value);
	void MoveRight(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);

	void Jump();
	void Crouch();
	void Sprint();
	void StopSprint();

	// Editable Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float PitchRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float JumpHeight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float SprintMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float StaminaWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
		float Stamina;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool IsRunning;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool RegStamina;
private:
	bool IsOnGround();
	void SetRegStaminaTrue();
};
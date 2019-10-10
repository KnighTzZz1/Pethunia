// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
UCLASS()
class PETHUNIA_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	APlayerCharacter();
	
	// Components
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Camera")
		USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* PlayerStaticMesh;

protected:
	virtual void BeginPlay() override;

	//Basic Stuff
	void MoveForward(float value);
	void MoveRight(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);

	void PlayerCrouch();
	void PlayerUncrouch();
	void PlayerProne();
	
	//Editable Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float PitchRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float JumpHeight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float MaxStamina = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float maxSpeed = 500.f;

	// Server-Client Functions
	UFUNCTION(Server,Reliable, WithValidation)
		void Server_PlayerJump();
	void Client_PlayerJump();
	
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlayerSprint();
	void Client_PlayerSprint();
	
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlayerStopSprint();
	void Client_PlayerStopSprint();

	
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedLocation)
		FVector ReplicatedLocation;
	UFUNCTION()
		void OnRep_ReplicatedLocation();

	// Replicated Variables
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		float Stamina = 500.f;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		bool isRunning;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		bool regStamina;
public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FTimerHandle StaminaRechargeTimer;
private:
	bool isOnGround();
	void regenerateStamina();
	
};
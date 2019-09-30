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
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Camera")
		USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* PlayerStaticMesh;
protected:
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);
	void PlayerJump();
	void PlayerCrouch();
	void PlayerSprint();
	void PlayerStopSprint();
	void PlayerProne();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float PitchRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float JumpHeight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
		float maxSpeed = 500.f;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float Stamina;
	FTimerHandle StaminaRechargeTimer;
private:
	bool isOnGround();
	bool isSprinting;
	bool regStamina;
	void regenerateStamina();
	
};
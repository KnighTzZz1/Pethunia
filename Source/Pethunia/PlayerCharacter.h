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
class UCapsuleComponent;

UCLASS()
class PETHUNIA_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	APlayerCharacter();
	// Components
	UPROPERTY(VisibleAnywhere, blueprintReadOnly, Category = "Player")
		UCapsuleComponent* TriggerCapsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UStaticMeshComponent* PlayerStaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UPlayerHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		UPlayerEnergyComponent* EnergyComponent;

	virtual void DashAbility() {};

	void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	FTimerHandle StaminaRechargeTimer;

protected:
	bool isSliding;

	void BeginPlay() override;

	// Movement
	void MoveForward(float value);
	void MoveRight(float value);
	void LookHorizontal(float value);
	void LookVertical(float value);

	void Jump();

	UFUNCTION(Server, reliable)
		void Server_Jump();

	virtual void CrouchStart() {};
	virtual void CrouchStop() {};

	void SprintStart();

	UFUNCTION(Server, reliable)
		void Server_SprintStart();

	void SprintStop();

	UFUNCTION(Server, reliable)
		void Server_SprintStop();

	virtual void Power1Activate() {};
	virtual void Power1Deactivate() {};

	virtual void Interact(AActor* ActorToInteract) {};
	virtual void LMB() {};
	virtual void Reload() {};
	virtual void DropWeapon() {};

	UFUNCTION()
		void TryToInteract();

public:	
	// Editable Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float TurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float PitchRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0", ClampMax = "2000"))
		float JumpHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0"))
		float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "1.0", ClampMax = "5.0"))
		float SprintSpeedMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float CrouchSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0"))
		float MaxStamina;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		float CurrentStamina;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0"))
		float StaminaWaitTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float StaminaRegenMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "5"))
		float StaminaCostOnSprint;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool IsRunning;
	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool RegStamina;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
		bool IsOnLadder;

	UFUNCTION()
		void OverlapBeginEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OverlapEndEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UPROPERTY(EditAnywhere, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "500.0"))
		float InteractionDistance;
private:

	void SetRegStaminaTrue();

	FVector CurrentLadderLocation;
	FVector CurrentLadderForwardVector;
	FVector CurrentLadderUpVector;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Curves/CurveFloat.h"
#include "Components/TimelineComponent.h"


#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UPlayerHealthComponent;
class UPlayerEnergyComponent;
class UCapsuleComponent;

static const float CAMERA_LAND_MAX_HEIGHT = -35;

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

	virtual void Landed(const FHitResult & Hit) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0"))
		float SlowAmmoundOnLand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "5.0"))
		float LandingSlowDuration;

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


	// Camera Shake - - - Walk - - -
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCurveFloat* CameraWalkCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCurveFloat* CameraJumpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCurveFloat* CameraLandingCurve;

	FRotator Camera_InitialRotation;
	FRotator Camera_TargetRotation;

	FRotator Camera_InitialJumpRotation;
	FRotator Camera_TargetJumpRotation;

	FRotator Camera_InitialLandRotation;
	FRotator Camera_TargetLandRotation;

	FVector Camera_InitialLocation;
	FVector Camera_TargetLocation;

	UPROPERTY(EditAnywhere, Category = "Camera")
		bool CameraRotateOnLand;
	UPROPERTY(EditAnywhere, Category = "Camera")
		bool CameraMoveOnLand;

	FVector Camera_InitialLandLocation;
	FVector Camera_TargetLandLocation;

	FTimeline CameraWalkTimeline;

	FTimeline CameraJumpTimeline;

	FTimeline CameraLandTimeline;

	UFUNCTION()
		void HandleCameraWalkProgress(float value);
	UFUNCTION()
		void HandleCameraJumpProgress(float value);
	UFUNCTION()
		void HandleCameraLandProgress(float value);

	UPROPERTY(EditAnywhere, Category = "Camera")
		float CameraOffset;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float CameraJumpOffset;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float CameraLandOffset;

	UPROPERTY(EditAnywhere, Category = "Camera")
		bool CameraJumpLoop;
	


	UFUNCTION()
		void SlowMovementOnLand();

	UFUNCTION()
		void RemoveLandingSlow();
private:

	bool resetJumpStartHeight;
	float JumpStartHeight;

	void SetRegStaminaTrue();
	 
	FVector CurrentLadderLocation;
	FVector CurrentLadderForwardVector;
	FVector CurrentLadderUpVector;
};
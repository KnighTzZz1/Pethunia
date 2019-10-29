// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "BasicBullet.generated.h"
#include "DrawDebugHelpers.h"
#include "PlayerHealthComponent.h"
#include "PlayerEnergyComponent.h"


APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("Player Health Component"));
	EnergyComponent = CreateDefaultSubobject<UPlayerEnergyComponent>(TEXT("Player Energy Component"));

	/*
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArm->TargetArmLength = 0;
	SpringArm->SetupAttachment(RootComponent);
	*/
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	TurnRate = 30.f;
	PitchRate = 30.f;
	JumpHeight = 45.f;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = NormalSpeed / 2;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (RegStamina && Stamina < MaxStamina)
	{
		Stamina += 1.f;
		if (Stamina == MaxStamina) RegStamina = false;
	}
	if (!RegStamina && IsRunning && Stamina > 0 )
	{
		Stamina -= 1.f;
		if (Stamina <= 0)
		{
			StopSprint();
		}
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator(0, ControllerRotation.Yaw, 0);

		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator(0,ControllerRotation.Yaw, 0);

		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::LookHorizontal(float Value)
{
	AddControllerYawInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookVertical(float Value)
{
	AddControllerPitchInput(Value * PitchRate * GetWorld()->GetDeltaSeconds());
}

bool APlayerCharacter::IsOnGround()
{
	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	
	GetController()->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
	FVector StartPosition = ViewPointLocation + FVector(0.f, 0.f, -88.f);
	FVector EndPosition = StartPosition + FVector(0.0f, 0.0f, -160.f);
	
	bool isHit = GetWorld()->LineTraceSingleByChannel(Hit, StartPosition, EndPosition, ECC_Visibility, TraceParams);
	return isHit;
}

void APlayerCharacter::Jump()
{
	if (IsOnGround() && Stamina > 100) 
	{
		RegStamina = false;
		Stamina -= 100.f;
		APlayerCharacter::LaunchCharacter(FVector(0.f, 0.f, JumpHeight), false, true);
		GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::SetRegStamina(true), 2.0f, false);
	}
}

void APlayerCharacter::Crouch()
{
	
}

void APlayerCharacter::SetRegStaminaTrue()
{
	RegStamina = true;
}

void APlayerCharacter::StartSprint()
{
	IsRunning = true;
	RegStamina = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SprintMultiplier;
}

void APlayerCharacter::StopSprint()
{
	IsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::RegenerateStamina, StaminaWaitTime, false);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::Crouch);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::StopSprint);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookHorizontal", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookVertical", this, &APawn::AddControllerPitchInput);
}
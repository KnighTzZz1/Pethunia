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
#include "Components/CapsuleComponent.h"

#define printout(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT(x));}


APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("Player Health Component"));
	EnergyComponent = CreateDefaultSubobject<UPlayerEnergyComponent>(TEXT("Player Energy Component"));
	this->AddInstanceComponent(EnergyComponent);

	TriggerCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Trigger Component"));
	TriggerCapsule->InitCapsuleSize(34.f, 88.f);
	TriggerCapsule->SetCollisionProfileName(TEXT("Trigger"));
	TriggerCapsule->SetupAttachment(RootComponent);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArm->TargetArmLength = 0;
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 81.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	PlayerStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	PlayerStaticMesh->SetupAttachment(RootComponent);
	TriggerCapsule->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OverlapBeginEvent);
	TriggerCapsule->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OverlapEndEvent);

	TurnRate = 30.f;
	PitchRate = 30.f;
	JumpHeight = 500.f;
	NormalSpeed = 600.f;
	MaxStamina = 500.f;
	CurrentStamina = MaxStamina;
	IsRunning = false;
	RegStamina = false;
	IsOnLadder = false;
	CrouchSpeedMultiplier = 0.5f;
	StaminaCostOnJump = 100.f;
	StaminaWaitTime = 2.f;
	StaminaRegenMultiplier = 1.f;
	StaminaCostOnSprint = 0.5f;
	SprintSpeedMultiplier = 1.8f;
	

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = NormalSpeed * CrouchSpeedMultiplier;
}


void APlayerCharacter::OverlapBeginEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		if (OtherActor->ActorHasTag(FName(TEXT("Ladder"))))
		{
			UE_LOG(LogTemp, Error, TEXT("Ladder Detected"));
			IsOnLadder = true;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			CurrentLadderLocation = OtherActor->GetActorLocation();
			
		}
		UE_LOG(LogTemp, Warning, TEXT("Overlaping"));
	}
}

void APlayerCharacter::OverlapEndEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && OtherComp)
	{
		if (OtherActor->ActorHasTag(FName(TEXT("Ladder"))))
		{
			IsOnLadder = false;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
		UE_LOG(LogTemp, Warning, TEXT("Stopped overlaping"));
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (RegStamina && CurrentStamina < MaxStamina)
	{
		CurrentStamina += StaminaRegenMultiplier;
		if (CurrentStamina == MaxStamina) RegStamina = false;
	}
	if (!RegStamina && IsRunning && CurrentStamina > 0 )
	{
		CurrentStamina -= StaminaCostOnSprint;
		if (CurrentStamina <= 0)
		{
			SprintStop();
		}
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{
		if (IsOnLadder)
		{
			UE_LOG(LogTemp, Warning, TEXT("Trying to move on ladder"));
			AddMovementInput(GetActorUpVector(), Value);
		}
		else
		{
			FRotator ControllerRotation = Controller->GetControlRotation();
			FRotator YawRotation = FRotator(0, ControllerRotation.Yaw, 0);

			FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f && !IsOnLadder)
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
	if (IsOnLadder)
	{
		FVector Distance = GetActorLocation() - CurrentLadderLocation;
		UE_LOG(LogTemp, Error, TEXT("Direction Is: %s"), *Distance.ToString());
		FVector Direction = FVector(Distance.X, Distance.Y, 0);
		APlayerCharacter::LaunchCharacter(Direction.GetSafeNormal() * 1000, false, true);
		
	}
	else if (IsOnGround() && CurrentStamina > StaminaCostOnJump)
	{
		RegStamina = false;
		CurrentStamina -= StaminaCostOnJump;
		APlayerCharacter::LaunchCharacter(FVector(0.f, 0.f, JumpHeight), false, true);
		GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::SetRegStaminaTrue, StaminaWaitTime, false);
	}
}

void APlayerCharacter::CrouchStart()
{
	Super::Crouch();
}

void APlayerCharacter::CrouchStop()
{
	Super::UnCrouch();
}

void APlayerCharacter::SetRegStaminaTrue()
{
	RegStamina = true;
}

void APlayerCharacter::SprintStart()
{
	IsRunning = true;
	RegStamina = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SprintSpeedMultiplier;
}

void APlayerCharacter::SprintStop()
{
	IsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::SetRegStaminaTrue, StaminaWaitTime, false);
}


// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchStart);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::CrouchStop);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::SprintStop);
	PlayerInputComponent->BindAction("Ability", IE_Pressed, this, &APlayerCharacter::DashAbility);
	PlayerInputComponent->BindAction("Power 1", IE_Pressed, this, &APlayerCharacter::Power1Activate);


	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookHorizontal", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookVertical", this, &APawn::AddControllerPitchInput);
}
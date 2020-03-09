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
	JumpHeight = 700;
	NormalSpeed = 500;
	MaxStamina = 500;
	CurrentStamina = MaxStamina;
	IsRunning = false;
	RegStamina = false;

	IsOnLadder = false;

	CrouchSpeedMultiplier = 0.5f;
	StaminaWaitTime = 2.f;
	StaminaRegenMultiplier = 0.5f;
	StaminaCostOnSprint = 0.25f;
	SprintSpeedMultiplier = 1.6f;
	

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
			IsOnLadder = true;
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			CurrentLadderLocation = OtherActor->GetActorLocation();
			CurrentLadderForwardVector = OtherActor->GetActorForwardVector();
			CurrentLadderUpVector = OtherActor->GetActorUpVector();
		}
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
	}
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	IsOnLadder = false;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (RegStamina && CurrentStamina < MaxStamina)
	{
		CurrentStamina += StaminaRegenMultiplier;
		if (CurrentStamina == MaxStamina) RegStamina = false;
	}
	if (!RegStamina && IsRunning && CurrentStamina > 0 && GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Size() != 0)
	{
		CurrentStamina -= StaminaCostOnSprint;
		if (CurrentStamina <= 0.5)
		{
			GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		}
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.f)
	{	
		if (!isSliding)
		{
			FRotator ControllerRotation = Controller->GetControlRotation();
			FRotator YawRotation = FRotator(0, ControllerRotation.Yaw, 0);

			FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			if (IsOnLadder)
			{
				float delta = Camera->GetForwardVector().Z;
				UE_LOG(LogTemp, Warning, TEXT("Direction: %f"), delta);
				if (delta >= -0.2f)
				{
					AddMovementInput(GetActorUpVector() * 1, Value);
				}
				else AddMovementInput(GetActorUpVector() * (-1), Value);
			}
			else
			{
				AddMovementInput(Direction, Value);
			}
		}
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f )
	{
		if (!isSliding)
		{
			FRotator ControllerRotation = Controller->GetControlRotation();
			FRotator YawRotation = FRotator(0, ControllerRotation.Yaw, 0);

			FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			if (IsOnLadder)
			{
				float a = FVector::DotProduct(CurrentLadderForwardVector, Direction);
				FVector NewDirection = CurrentLadderForwardVector * a;
				AddMovementInput(NewDirection, Value);
			}
			else AddMovementInput(Direction, Value);
		}
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


void APlayerCharacter::Jump()
{
	if (IsOnLadder)
	{
		FVector Distance = GetActorLocation() - CurrentLadderLocation;
		FVector Direction = FVector(Distance.X, Distance.Y, 0);
		APlayerCharacter::LaunchCharacter(Direction.GetSafeNormal() * 1000, false, true);
	}
	else if (GetCharacterMovement()->IsMovingOnGround())
	{
		APlayerCharacter::LaunchCharacter(FVector(0.f, 0.f, JumpHeight), false, true);
	}
}

void APlayerCharacter::SetRegStaminaTrue()
{
	RegStamina = true;
}

void APlayerCharacter::SprintStart()
{
	if (CurrentStamina <= 0) return;
	GetWorldTimerManager().ClearTimer(StaminaRechargeTimer);
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
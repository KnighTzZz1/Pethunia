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



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArm->TargetArmLength = 0;
	SpringArm->SetupAttachment(RootComponent);
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	PlayerStaticMesh = CreateDefaultSubobject <UStaticMeshComponent>(TEXT("Static Mesh"));
	PlayerStaticMesh->SetupAttachment(RootComponent);
	TurnRate = 30.f;
	PitchRate = 30.f;
	JumpHeight = 45.f;
	GetCharacterMovement()->MaxWalkSpeed = maxSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = maxSpeed / 2;
	
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacter, regStamina);
	DOREPLIFETIME(APlayerCharacter, isRunning);
	DOREPLIFETIME(APlayerCharacter, Stamina);
	DOREPLIFETIME(APlayerCharacter, ReplicatedLocation);

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCharacter::OnRep_ReplicatedLocation()
{
	SetActorLocation(ReplicatedLocation);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		ReplicatedLocation = GetActorLocation();
	}
	if (regStamina && Stamina < MaxStamina)
	{
		Stamina += 1.f;
		if (Stamina == MaxStamina) regStamina = false;
	}
	if (!regStamina && isRunning && Stamina > 0 )
	{
		Stamina -= 1.f;
	}
	
}

void APlayerCharacter::MoveForward(float value)
{
	if (Controller && value != 0.f)
	{
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator(0, ControllerRotation.Yaw, 0);

		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if (Controller && value != 0.0f)
	{
		FRotator ControllerRotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator(0,ControllerRotation.Yaw, 0);

		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}

void APlayerCharacter::LookHorizontal(float value)
{
	AddControllerYawInput(value * TurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookVertical(float value)
{
	AddControllerPitchInput(value * PitchRate * GetWorld()->GetDeltaSeconds());
}

bool APlayerCharacter::isOnGround()
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

void APlayerCharacter::Client_PlayerJump()
{
	Server_PlayerJump();
}

void APlayerCharacter::Server_PlayerJump_Implementation()
{
	if (isOnGround() && Stamina > 0) 
	{
		regStamina = false;
		Stamina -= 100.f;
		APlayerCharacter::LaunchCharacter(FVector(0.f, 0.f, JumpHeight), false, true);
		GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::regenerateStamina, 2.0f, false);
	}
}



bool APlayerCharacter::Server_PlayerJump_Validate()
{
	return true;
}

void APlayerCharacter::PlayerCrouch()
{
	Crouch();

}
void APlayerCharacter::PlayerUncrouch()
{
	UnCrouch();
}

void APlayerCharacter::PlayerProne()
{
	
}


void APlayerCharacter::regenerateStamina()
{
	regStamina = true;
}

void APlayerCharacter::Client_PlayerSprint()
{
	
	GetCharacterMovement()->MaxWalkSpeed = maxSpeed * 1.5;
	Server_PlayerSprint();
}

void APlayerCharacter::Server_PlayerSprint_Implementation()
{
	isRunning = true;
	regStamina = false;
	GetCharacterMovement()->MaxWalkSpeed = maxSpeed * 1.5;
}

bool APlayerCharacter::Server_PlayerSprint_Validate()
{
	return true;
}

void APlayerCharacter::Client_PlayerStopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = maxSpeed;
	Server_PlayerStopSprint();
}

void APlayerCharacter::Server_PlayerStopSprint_Implementation()
{
	isRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = maxSpeed;
	GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::regenerateStamina, 2.0f, false);
}

bool APlayerCharacter::Server_PlayerStopSprint_Validate()
{
	return true;
}



// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Client_PlayerJump);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::PlayerCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::PlayerUncrouch);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::Client_PlayerSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::Client_PlayerStopSprint);
	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &APlayerCharacter::PlayerProne);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookHorizontal", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookVertical", this, &APawn::AddControllerPitchInput);
}



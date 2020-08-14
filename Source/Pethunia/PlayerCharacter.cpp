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




APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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

	resetJumpStartHeight = true;

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
			UE_LOG(LogTemp, Warning, TEXT("Miekari Ladders"));
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

	FOnTimelineFloat CameraWalkFunction;
	FOnTimelineFloat CameraJumpFunction;
	FOnTimelineFloat CameraLandFunction;

	CameraWalkFunction.BindUFunction(this, FName("HandleCameraWalkProgress"));
	CameraJumpFunction.BindUFunction(this, FName("HandleCameraJumpProgress"));
	CameraLandFunction.BindUFunction(this, FName("HandleCameraLandProgress"));

	CameraWalkTimeline.AddInterpFloat(CameraWalkCurve, CameraWalkFunction);
	CameraWalkTimeline.SetLooping(true);

	CameraJumpTimeline.AddInterpFloat(CameraJumpCurve, CameraJumpFunction);
	CameraJumpTimeline.SetLooping(CameraJumpLoop);

	CameraLandTimeline.AddInterpFloat(CameraLandingCurve, CameraLandFunction);
	CameraJumpTimeline.SetLooping(false);

	Camera_InitialRotation = Camera->GetRelativeTransform().GetRotation().Rotator();
	Camera_TargetRotation = Camera_InitialRotation + FRotator(CameraOffset,0,0);
	
	Camera_InitialLocation = SpringArm->GetRelativeTransform().GetLocation();
	Camera_TargetLocation = Camera_InitialLocation + FVector(0, 0, CameraOffset);

	Camera_InitialJumpRotation = Camera->GetRelativeTransform().GetRotation().Rotator();
	Camera_TargetJumpRotation = Camera_InitialJumpRotation + FRotator(CameraJumpOffset, 0, 0);

	Camera_InitialLandRotation = Camera->GetRelativeTransform().GetRotation().Rotator();
	Camera_TargetLandRotation = FRotator::ZeroRotator + FRotator(CameraLandOffset, 0, 0);
	
	Camera_InitialLandLocation = Camera->GetRelativeTransform().GetLocation();

	CameraWalkTimeline.PlayFromStart();
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
	if (GetCharacterMovement()->Velocity != FVector	::ZeroVector && !isSliding && !IsOnLadder && !bIsCrouched && GetCharacterMovement()->IsMovingOnGround())
	{
		CameraWalkTimeline.TickTimeline(DeltaTime);
	}
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		CameraJumpTimeline.TickTimeline(DeltaTime);
	}
	if (resetJumpStartHeight && !GetCharacterMovement()->IsMovingOnGround())
	{
		resetJumpStartHeight = false;
		JumpStartHeight = GetGameTimeSinceCreation();
	}
			
	CameraLandTimeline.TickTimeline(DeltaTime);
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
			else if (!GetCharacterMovement()->IsMovingOnGround())
			{
				AddMovementInput(Direction * 0.3, Value);
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


void APlayerCharacter::SlowMovementOnLand(float delta)
{
	float pass = delta;
	GetCharacterMovement()->MaxWalkSpeed -= pass * SlowAmmoundOnLand;
	FTimerHandle LandingTimer;
	FTimerDelegate LandDelegate = FTimerDelegate::CreateUObject(this, &APlayerCharacter::RemoveLandingSlow, pass);
	GetWorldTimerManager().SetTimer(LandingTimer, LandDelegate, LandingSlowDuration, false);
}

void APlayerCharacter::RemoveLandingSlow(float pass)
{
	UE_LOG(LogTemp, Warning, TEXT("REMOVED SLOW"));
	GetCharacterMovement()->MaxWalkSpeed += pass * SlowAmmoundOnLand;
}

void APlayerCharacter::Landed(const FHitResult & Hit)
{
	Super::Landed(Hit);
	UE_LOG(LogTemp, Warning, TEXT("ADDED SLOW"));

	resetJumpStartHeight = true;
	float LandHeight = GetGameTimeSinceCreation();
	SlowMovementOnLand(LandHeight - JumpStartHeight);


	Camera_TargetLandLocation = FVector::ZeroVector + FVector(0, 0, CameraLandOffset * (LandHeight - JumpStartHeight));
	if (Camera_TargetLandLocation.Z < CAMERA_LAND_MAX_HEIGHT) Camera_TargetLandLocation.Z = CAMERA_LAND_MAX_HEIGHT;
	CameraLandTimeline.PlayFromStart();
}


void APlayerCharacter::Server_Jump_Implementation()
{
	Jump();
}

void APlayerCharacter::Jump()
{
	if (!HasAuthority())
	{
		Server_Jump();
	}
	
	if (IsOnLadder)
	{
		FVector Distance = GetActorLocation() - CurrentLadderLocation;
		FVector Direction = FVector(Distance.X, Distance.Y, 0);
		APlayerCharacter::LaunchCharacter(Direction.GetSafeNormal() * 1000, false, true);
	}
	else if (GetCharacterMovement()->IsMovingOnGround())
	{
		APlayerCharacter::LaunchCharacter(FVector(0.f, 0.f, JumpHeight), false, true);
		CameraJumpTimeline.PlayFromStart();
	}
}

void APlayerCharacter::SetRegStaminaTrue()
{
	RegStamina = true;
}

void APlayerCharacter::Server_SprintStart_Implementation()
{
	SprintStart();
}

void APlayerCharacter::SprintStart()
{
	if (!HasAuthority())
	{
		Server_SprintStart();
	}

	if (CurrentStamina <= 0) return;
	GetWorldTimerManager().ClearTimer(StaminaRechargeTimer);
	IsRunning = true;
	RegStamina = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SprintSpeedMultiplier;

}

void APlayerCharacter::Server_SprintStop_Implementation()
{
	SprintStop();
}

void APlayerCharacter::SprintStop()
{
	if (!HasAuthority())
		Server_SprintStop();
	IsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetWorldTimerManager().SetTimer(StaminaRechargeTimer, this, &APlayerCharacter::SetRegStaminaTrue, StaminaWaitTime, false);
}


void APlayerCharacter::TryToInteract()
{
	FHitResult Hit;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * InteractionDistance;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bTraceComplex = true;

	bool hasHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End,ECollisionChannel::ECC_Visibility, CollisionParams);
	if (hasHit)
	{
		AActor* HitActor = Hit.GetActor();
		Interact(HitActor);		
		DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 2, false);
	} 
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2, false);
	}
}

void APlayerCharacter::HandleCameraWalkProgress(float value)
{
	FRotator NewRotation = FMath::Lerp(Camera_InitialRotation, Camera_TargetRotation, value);
	Camera->SetRelativeRotation(NewRotation);

	FVector NewLocation = FMath::Lerp(Camera_InitialLocation, Camera_TargetLocation, value);
	SpringArm->SetRelativeLocation(NewLocation);
}

void APlayerCharacter::HandleCameraJumpProgress(float value)
{
	FRotator NewRotation = FMath::Lerp(Camera_InitialJumpRotation, Camera_TargetJumpRotation, value);
	Camera->SetRelativeRotation(NewRotation);
}

void APlayerCharacter::HandleCameraLandProgress(float value)
{
	if (CameraRotateOnLand)
	{
		FRotator NewRotation = FMath::Lerp(Camera_InitialLandRotation, Camera_TargetLandRotation, value);
		Camera->SetRelativeRotation(NewRotation);
	}
	if (CameraMoveOnLand)
	{
		FVector NewLocation = FMath::Lerp(Camera_InitialLandLocation, Camera_TargetLandLocation, value);
		Camera->SetRelativeLocation(NewLocation);
	}
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
	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &APlayerCharacter::TryToInteract);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::LMB);
	PlayerInputComponent->BindAction("Reload",IE_Pressed,this,&APlayerCharacter::Reload);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &APlayerCharacter::DropWeapon);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookHorizontal", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookVertical", this, &APawn::AddControllerPitchInput);
}



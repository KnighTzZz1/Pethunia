// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthCharacter.h"
#include "PlayerEnergyComponent.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "Gun.h"
#include "EngineGlobals.h"
#include "Components/StaticMeshComponent.h"

#define print(text) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,1.5f,FColor::Green, TEXT(text));

AStealthCharacter::AStealthCharacter()
{
	IncreasedEnergyMultiplier = 1.3f;
	PowerDuration = 5.0f;
	PowerCooldown = 10.0f;
	PowerCost = 40.f;
	
	// Dash
	DashStopTime = 0.05f;
	DashCooldownTime = 3.0f;
	DashSpeed = 17000.0f;
	DashCost = 20.0f;

	// Sliding
	VanillaDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
	VanillaGroundFriction = GetCharacterMovement()->GroundFriction;
	SlideDeceleration = 0;
	SlideGroundFriction = 0.5f;
	SlideBoost = 100.f;
	SlideDelay = 0.3f;


	ActiveWeapon = nullptr;
	isReloading = false;
	
}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSlidingSpeed();
	
}	

void AStealthCharacter::DashAbility()
{
	if (EnergyComponent->CurrentEnergy >= DashCost && !DashIsOnCooldown)
	{
		EnergyComponent->UseEnergy(DashCost);
		FVector Direction = Camera->GetForwardVector() * DashSpeed;
		GetCharacterMovement()->Launch(Direction);
		DashIsOnCooldown = true;
		GetWorldTimerManager().SetTimer(DashHandle, this, &AStealthCharacter::StopMovement, DashStopTime, false);
		
	}

}

void AStealthCharacter::StopMovement()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(DashHandle, this, &AStealthCharacter::SetDashCooldownOff, DashCooldownTime, false);
}

void AStealthCharacter::SetDashCooldownOff()
{
	DashIsOnCooldown = false;
}

void AStealthCharacter::Power1Activate()
{
	
	if (bPower1IsOnCooldown == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Power Activated"));
		if (EnergyComponent && EnergyComponent->CurrentEnergy - PowerCost >= 0)
		{
			EnergyComponent->UseEnergy(PowerCost);

			SprintSpeedMultiplier *= 1.2f;
			JumpHeight *= 1.5;
			bPower1IsOnCooldown = true;
			GetWorldTimerManager().SetTimer(Power1Handle, this, &AStealthCharacter::Power1Deactivate, PowerDuration, false);
			
		}
	}
}

void AStealthCharacter::Power1Deactivate()
{
	UE_LOG(LogTemp, Error, TEXT("Power Deactivated"));
	if (EnergyComponent) {
		GetWorldTimerManager().SetTimer(Power1CooldownHandle, this, &AStealthCharacter::PowerCooldownOff, PowerCooldown, false);
		EnergyComponent->DashCost /= IncreasedEnergyMultiplier;
		SprintSpeedMultiplier /= 1.2f;
		JumpHeight /= 1.5;
	}
}

void AStealthCharacter::PowerCooldownOff()
{
	bPower1IsOnCooldown = false;
}

void AStealthCharacter::CrouchStart()
{
	if (IsRunning && !SlideIsOnCooldown)
	{
		FVector velocity = GetCharacterMovement()->Velocity;
		float res = velocity.Size();
		UE_LOG(LogTemp, Warning, TEXT("Velocity: %f"), res);
		if (res >= 600)
		{

			if (GetCharacterMovement()->IsMovingOnGround())
			{
				FVector Direction = GetActorForwardVector();
				GetCharacterMovement()->Launch(Direction * SlideBoost);
			}

			GetCharacterMovement()->GroundFriction = 0.5;
			GetCharacterMovement()->BrakingDecelerationWalking = 0;
			isSliding = true;
		}

	}
	Super::Crouch();
}

void AStealthCharacter::CrouchStop()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		SlideIsOnCooldown = true;
		GetWorldTimerManager().SetTimer(SlideHandle, this, &AStealthCharacter::SlideCooldownOff, SlideDelay, false);
	}
	isSliding = false;
	GetCharacterMovement()->GroundFriction = VanillaGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = VanillaDeceleration;
	Super::UnCrouch();
}

bool AStealthCharacter::shouldUpdateAngle()
{
	if (GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Size() > 0)
	{
		return true;
	}
	return false;
}
	
float AStealthCharacter::GetSlidingAngle()
{
	FRotator result = UKismetMathLibrary::FindLookAtRotation(LastLocation,GetActorLocation());
	LastLocation = GetActorLocation();
	return result.Pitch;
}

void AStealthCharacter::UpdateSlidingSpeed()
{
	if (shouldUpdateAngle())
	{
		float angle = GetSlidingAngle();
		if (angle < 0 && angle >= -45)
		{
			if (SlideCurve)
			{
				float impulse = SlideCurve->GetFloatValue(angle);
				FVector force = GetActorForwardVector() * impulse;
				GetCharacterMovement()->AddImpulse(force,false);
			}
		}

	}
}

void AStealthCharacter::SlideCooldownOff()
{
	SlideIsOnCooldown = false;
}

void AStealthCharacter::Interact(AActor* ActorToInteract)
{
	if (ActorToInteract->ActorHasTag(FName(TEXT("Weapon"))))
	{
		if (WeaponInventory.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponInventory is Empty"));
			ActiveWeapon = (AGun*)ActorToInteract;
			
		}
		WeaponInventory.Add((AGun*)ActorToInteract);
		ActorToInteract->SetActorHiddenInGame(true);
		ActorToInteract->SetActorTickEnabled(false);
		ActorToInteract->SetActorEnableCollision(false);
		UStaticMeshComponent* act = (UStaticMeshComponent*)ActorToInteract->GetRootComponent();
		act->SetSimulatePhysics(false);
	}
}

void AStealthCharacter::AddWeaponToInventory(AGun* weapon)
{
	WeaponInventory.Add(weapon);
}

void AStealthCharacter::LMB()
{
	if (!ActiveWeapon) return;
	if (isReloading) return;
	if (ActiveWeapon->CurrentAmmo > 0)
	{
		FHitResult Hit;
		FVector Start = Camera->GetComponentLocation();
		FVector End = Start + Camera->GetForwardVector() * ActiveWeapon->ShootDistance;
		bool hasHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility);
		if (hasHit)
		{
			DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 2, false);
		}
		ActiveWeapon->CurrentAmmo--;
	}
}

void AStealthCharacter::Reload()
{
	if (!ActiveWeapon) return;
	if (ActiveWeapon->CurrentAmmo == ActiveWeapon->MaxAmmo || ActiveWeapon->NumberOfMagazines == 0) return;
	isReloading = true;
	GetWorldTimerManager().SetTimer(ReloadTime, this, &AStealthCharacter::ReloadWeapon, ActiveWeapon->ReloadTime);
}

void AStealthCharacter::ReloadWeapon()
{
	ActiveWeapon->CurrentAmmo = ActiveWeapon->MaxAmmo;
	ActiveWeapon->NumberOfMagazines--;
	isReloading = false;
}

void AStealthCharacter::DropWeapon()
{
	if (!ActiveWeapon) return; // If I don't have anything equiped, then I can't drop anything.
	int index = WeaponInventory.IndexOfByKey(ActiveWeapon); // Get the index in inventory.
	UE_LOG(LogTemp, Warning, TEXT("Index: %d"), index);
	WeaponInventory.RemoveAt(index);
	ActiveWeapon->SetActorLocation(GetActorLocation());
	ActiveWeapon->SetActorHiddenInGame(false);
	ActiveWeapon->SetActorEnableCollision(true);
	ActiveWeapon->SetActorTickEnabled(true);
	UStaticMeshComponent* act = (UStaticMeshComponent*)ActiveWeapon->GetRootComponent();
	act->SetSimulatePhysics(true);
	ActiveWeapon = nullptr;
}
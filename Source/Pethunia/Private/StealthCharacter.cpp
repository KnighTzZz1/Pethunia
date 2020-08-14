// Fill out your copyright notice in the Description page of Project Settings.


#include "StealthCharacter.h"
#include "PlayerEnergyComponent.h"
#include "PlayerHealthComponent.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "Gun.h"
#include "EngineGlobals.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

#include "Net/UnrealNetwork.h"

#include "Pethunia.h"

#define print(text) if(GEngine) GEngine->AddOnScreenDebugMessage(-1,1.5f,FColor::Green, TEXT(text));
#define printVector(vector) UE_LOG(LogTemp,Warning,TEXT("Vector: %s"),*vector.ToString());

AStealthCharacter::AStealthCharacter()
{
	bReplicates = true;

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

	inv.Add(1, nullptr);
	inv.Add(2, nullptr);

	ActiveWeapon = nullptr;
	
	
	Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Player Arms"));
	PlayerWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Player Weapon"));
	Arms->SetupAttachment(Camera);
	
	//PlayerWeapon->AttachToComponent(Arms, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative,true), FName(TEXT("R_GunSocket")));


}

void AStealthCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSlidingSpeed();
	
}	

void AStealthCharacter::Server_DashAbility_Implementation()
{
	DashAbility();
}

void AStealthCharacter::DashAbility()
{
	if (!HasAuthority())
	{
		Server_DashAbility();
		return;
	}
	
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

void AStealthCharacter::Server_CrouchStart_Implementation()
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
	
	Server_CrouchStart();
}

void AStealthCharacter::Server_CrouchStop_Implementation()
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
	Server_CrouchStop();
}

bool AStealthCharacter::shouldUpdateAngle()
{
	if (GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->Velocity.Size() > 0 && isSliding)
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

void AStealthCharacter::Server_UpdateSlidingSpeed_Implementation(FVector force)
{
	GetCharacterMovement()->AddImpulse(force, false);
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
				Server_UpdateSlidingSpeed(force);
			}
		}
	}
	
}

void AStealthCharacter::SlideCooldownOff()
{
	SlideIsOnCooldown = false;
}

void AStealthCharacter::Server_Interact_Implementation(AActor* ActorToInteract)
{
	Interact(ActorToInteract);
}

// FIXME
void AStealthCharacter::Interact(AActor* ActorToInteract)
{
	if (!HasAuthority())
	{
		Server_Interact(ActorToInteract);
	}

	if (ActorToInteract->ActorHasTag(FName(TEXT("Weapon"))))
	{
		TryPickingUpWeapon((AGun*)ActorToInteract);
	}
	
}
// FIXME
void AStealthCharacter::TryPickingUpWeapon(AGun* weapon)
{
	if (weapon->GunOwner) return;
	if (*inv.Find(1) == nullptr && *inv.Find(2) == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Weapon"));
		inv.Add(1, weapon);
		ActiveWeapon = weapon;
		ActiveWeapon->GunOwner = this;
		UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *this->GetName());
		ActiveWeapon->Owner_Camera = Camera;
		SetupAnims();
	}
	else if (*inv.Find(1) == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Primary"));
		inv.Add(1, weapon);
		weapon->GunOwner = this;
		weapon->Owner_Camera = Camera;
		PutWeaponOnBack(weapon);
	}
	else if (*inv.Find(2) == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Secondary"));
		inv.Add(2, weapon);

		weapon->GunOwner = this;
		weapon->Owner_Camera = Camera;
		PutWeaponOnBack(weapon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Both Weapons"));
		return;
	}
}

void AStealthCharacter::LMB()
{
	if (!HasAuthority())
	{
		Server_LMB();
	}

	if (!ActiveWeapon) return;
	if (ActiveWeapon->isReloading) return;
	
	ActiveWeapon->FireWeapon(Arms);
}

void AStealthCharacter::Server_LMB_Implementation()
{
	LMB();
}

bool AStealthCharacter::Server_LMB_Validate()
{
	return true;
}

void AStealthCharacter::LMB_Released()
{
	if (!ActiveWeapon) return;	
	ActiveWeapon->StopFire();
}

void AStealthCharacter::ChangeFireMode()
{
	if (!ActiveWeapon) return;
	if (ActiveWeapon->isFiring) return;
	ActiveWeapon->ChangeFireMode();
}

void AStealthCharacter::Reload()
{
	if (!HasAuthority())
	{
		Server_Reload();
	}

	if (!ActiveWeapon) return;
	ActiveWeapon->ReloadWeapon(Arms);
}

void AStealthCharacter::Server_Reload_Implementation()
{
	Reload();
}

bool AStealthCharacter::Server_Reload_Validate()
{
	return true;
}


// FIXME
void AStealthCharacter::DropWeapon()
{
	if (!HasAuthority())
	{
		Server_DropWeapon();	
		
	}
	if (!ActiveWeapon) return; // If I don't have anything equiped, then I can't drop anything.

	if (ActiveWeapon->isFiring || ActiveWeapon->isReloading) return;

	if (*inv.Find(1) == ActiveWeapon)
	{
		inv.Add(1, nullptr);
		ClearAnims();
	}
	else
	{
		inv.Add(2, nullptr);
		ClearAnims();
	}
	
	ActiveWeapon->SetActorLocation(Camera->GetComponentLocation() + Camera->GetForwardVector() * 100);


	if (HasAuthority())
	{
		ActiveWeapon->UpdateGunDropLocation(Camera->GetComponentLocation() + Camera->GetForwardVector() * 100);
	}


	//ActiveWeapon->UpdateGunPosition();
	ActiveWeapon->RemoveOwnership();
	ActiveWeapon = nullptr; // Unequipe the gun
	
}


// FIXME
void AStealthCharacter::Server_DropWeapon_Implementation()
{
	DropWeapon();
}

bool AStealthCharacter::Server_DropWeapon_Validate()
{
	return true;
}


// FIXME
void AStealthCharacter::EquipPrimary()
{
	if (!HasAuthority())
	{
		Server_EquipPrimary();
	}

	if (*inv.Find(1) == nullptr)
	{
		print("No primary weapon");
		return;
	}
	if (ActiveWeapon)
	{
		if (ActiveWeapon->isFiring || ActiveWeapon->isReloading) return;
	}

	if (ActiveWeapon == *inv.Find(1)) return;

	ActiveWeapon = *inv.Find(1);
	SetupAnims();
	if (*inv.Find(2) == nullptr) return;
	PutWeaponOnBack(*inv.Find(2));

}
// FIXME
void AStealthCharacter::Server_EquipPrimary_Implementation()
{
	EquipPrimary();
}

// FIXME
void AStealthCharacter::EquipSecondary()
{
	if (!HasAuthority())
	{
		Server_EquipSecondary();
	}
	if (*inv.Find(2) == nullptr)
	{
		print("No secondary weapon");
		return;
	}
	if (ActiveWeapon)
	{
		if (ActiveWeapon->isFiring || ActiveWeapon->isReloading) return;
	}

	if (ActiveWeapon == *inv.Find(2)) return;

	ActiveWeapon = *inv.Find(2);

	SetupAnims();

	if (*inv.Find(1) == nullptr) return;
	PutWeaponOnBack(*inv.Find(1));
	
}
// FIXME
void AStealthCharacter::Server_EquipSecondary_Implementation()
{
	EquipSecondary();
}



void AStealthCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("EquipWeapon1",IE_Pressed, this, &AStealthCharacter::EquipPrimary);
	PlayerInputComponent->BindAction("EquipWeapon2", IE_Pressed, this, &AStealthCharacter::EquipSecondary);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStealthCharacter::LMB_Released);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &AStealthCharacter::ChangeFireMode);
}

void AStealthCharacter::GetLifetimeReplicatedProps(TArray < FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStealthCharacter, ActiveWeapon);

	
}


void AStealthCharacter::Server_Die_Implementation()
{
	Die();
}

void AStealthCharacter::Die()
{
	if (!HasAuthority())
	{
		Server_Die();
	}

	this->Destroy();
}

//---------------------- T O D O -----------------------//
/*
	
	StealthCharacter Networking Support
		[FIX - Equip]
	Gun Idle State FX
*/


//---------------------- D O N E -----------------------//
/*
	Gun Idle State Animations - DONE

	Gun PickUp when Overlapping - DONE

	Add Camera Shakes - DONE
*/
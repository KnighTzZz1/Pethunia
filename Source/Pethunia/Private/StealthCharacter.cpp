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
	if (EnergyComponent->CurrentEnergy >= DashCost && !DashIsOnCooldown)
	{
		EnergyComponent->UseEnergy(DashCost);
		FVector Direction = Camera->GetForwardVector() * DashSpeed;
		GetCharacterMovement()->Launch(Direction);
		DashIsOnCooldown = true;
		GetWorldTimerManager().SetTimer(DashHandle, this, &AStealthCharacter::StopMovement, DashStopTime, false);
	}
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
	Server_DashAbility();
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
	//if (ActorToInteract->ActorHasTag(FName(TEXT("Weapon"))))
	//{

	//	if (*inv.Find(1) == nullptr && *inv.Find(2) == nullptr)
	//	{
	//		inv.Add(1, (AGun*)ActorToInteract);
	//		ActiveWeapon = (AGun*)ActorToInteract;
	//		SetupAnims();
	//	}
	//	else if (*inv.Find(1) == nullptr)
	//	{
	//		inv.Add(1, (AGun*)ActorToInteract);
	//		PutWeaponOnBack(ActorToInteract);
	//	}
	//	else if (*inv.Find(2) == nullptr)
	//	{
	//		inv.Add(2, (AGun*)ActorToInteract);
	//		PutWeaponOnBack(ActorToInteract);
	//	}
	//	else
	//	{
	//		return;
	//	}
	//	
	//}
}

void AStealthCharacter::Interact(AActor* ActorToInteract)
{

	if (ActorToInteract->ActorHasTag(FName(TEXT("Weapon"))))
	{
		if (*inv.Find(1) == nullptr && *inv.Find(2) == nullptr)
		{
			print("Vapshe Ar Mqonda iaragi");
			inv.Add(1, (AGun*)ActorToInteract);
			ActiveWeapon = (AGun*)ActorToInteract;
			SetupAnims();
		}
		else if (*inv.Find(1) == nullptr)
		{
			print("Pirveli Carieli iko");
			inv.Add(1, (AGun*)ActorToInteract);
			PutWeaponOnBack(ActorToInteract);
		}
		else if(*inv.Find(2) == nullptr)
		{
			print("Meore Carieli iko");
			inv.Add(2, (AGun*)ActorToInteract);
			PutWeaponOnBack(ActorToInteract);
		}
		else
		{
			print("Oriveshi mejira");
			return;
		}
	}
}

void AStealthCharacter::Multi_Interact_Implementation(AActor* ActorToInteract)
{
	//if (ActorToInteract->ActorHasTag(FName(TEXT("Weapon"))))
	//{

	//	if (*inv.Find(1) == nullptr && *inv.Find(2) == nullptr)
	//	{
	//		inv.Add(1, (AGun*)ActorToInteract);
	//		ActiveWeapon = (AGun*)ActorToInteract;
	//		SetupAnims();
	//	}
	//	else if (*inv.Find(1) == nullptr)
	//	{
	//		inv.Add(1, (AGun*)ActorToInteract);
	//		PutWeaponOnBack(ActorToInteract);
	//	}
	//	else if (*inv.Find(2) == nullptr)
	//	{
	//		inv.Add(2, (AGun*)ActorToInteract);
	//		PutWeaponOnBack(ActorToInteract);
	//	}
	//	else
	//	{
	//		return;
	//	}
	//}
}

void AStealthCharacter::LMB()
{
	if (!ActiveWeapon) return;
	
	FHitResult Hit;
	ActiveWeapon->FireWeapon(&Hit, Camera, Arms);
}


void AStealthCharacter::LMB_Released()
{
	if (!ActiveWeapon) return;	
	ActiveWeapon->StopFire();
}

void AStealthCharacter::ChangeFireMode()
{
	if (!ActiveWeapon) return;
	ActiveWeapon->ChangeFireMode();
}

void AStealthCharacter::Reload()
{
	if (!ActiveWeapon) return;
	ActiveWeapon->ReloadWeapon(Arms);
}

void AStealthCharacter::DropWeapon()
{
	if (HasAuthority())
	{
		Multi_DropWeapon();
		return;
	}

	if (!ActiveWeapon) return; // If I don't have anything equiped, then I can't drop anything.

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
	Server_DropWeapon();
	ActiveWeapon = nullptr; // Unequipe the gun
	
}

void AStealthCharacter::Server_DropWeapon_Implementation()
{
	// if (*inv.Find(1) == ActiveWeapon)
	// {
	// 	inv.Add(1, nullptr);
	// 	ClearAnims();
	// }
	// else
	// {
	// 	inv.Add(2, nullptr);
	// 	ClearAnims();
	// }

	// ActiveWeapon->SetActorLocation(Camera->GetComponentLocation() + Camera->GetForwardVector() * 100);

	// ActiveWeapon = nullptr; // Unequipe the gun
}

void AStealthCharacter::Multi_DropWeapon_Implementation()
{
	// if (!ActiveWeapon) return; // If I don't have anything equiped, then I can't drop anything.

	// if (*inv.Find(1) == ActiveWeapon)
	// {
	// 	inv.Add(1, nullptr);
	// 	ClearAnims();
	// }
	// else
	// {
	// 	inv.Add(2, nullptr);
	// 	ClearAnims();
	// }

	// ActiveWeapon->SetActorLocation(Camera->GetComponentLocation() + Camera->GetForwardVector() * 100);

	// ActiveWeapon = nullptr; // Unequipe the gun
}

void AStealthCharacter::EquipPrimary()
{

	if (*inv.Find(1) == nullptr)
	{
		print("No primary weapon");
		return;
	}
	if (ActiveWeapon == *inv.Find(1)) return;
	
	ActiveWeapon = *inv.Find(1);
	SetupAnims();
	if (*inv.Find(2) == nullptr) return;
	PutWeaponOnBack(*inv.Find(2));

}

void AStealthCharacter::Server_EquipPrimary_Implementation()
{
	// if (*inv.Find(1) == nullptr)
	// {
	// 	return;
	// }
	// if (ActiveWeapon == *inv.Find(1)) return;
	// ActiveWeapon = *inv.Find(1);
	// SetupAnims();
	// if (*inv.Find(2) == nullptr) return;
	// PutWeaponOnBack(*inv.Find(2));
}

void AStealthCharacter::Multi_EquipPrimary_Implementation()
{
	// if (*inv.Find(1) == nullptr)
	// {
	// 	print("No primary weapon");
	// 	return;
	// }
	// if (ActiveWeapon == *inv.Find(1)) return;
	// ActiveWeapon = *inv.Find(1);
	// SetupAnims();
	// if (*inv.Find(2) == nullptr) return;
	// PutWeaponOnBack(*inv.Find(2));
}

void AStealthCharacter::EquipSecondary()
{

	if (*inv.Find(2) == nullptr)
	{
		print("No secondary weapon");
		return;
	}
	if (ActiveWeapon == *inv.Find(2)) return;
	
	ActiveWeapon = *inv.Find(2);
	SetupAnims();
	
	if (*inv.Find(1) == nullptr) return;
	PutWeaponOnBack(*inv.Find(1));

}

void AStealthCharacter::Server_EquipSecondary_Implementation()
{
	// if (*inv.Find(2) == nullptr)
	// {
	// 	return;
	// }
	// if (ActiveWeapon == *inv.Find(2)) return;
	// ActiveWeapon = *inv.Find(2);
	// SetupAnims();

	// if (*inv.Find(1) == nullptr) return;
	// PutWeaponOnBack(*inv.Find(1));
}

void AStealthCharacter::Multi_EquipSecondary_Implementation()
{
	// if (*inv.Find(2) == nullptr)
	// {
	// 	return;
	// }
	// if (ActiveWeapon == *inv.Find(2)) return;
	// ActiveWeapon = *inv.Find(2);
	// SetupAnims();

	// if (*inv.Find(1) == nullptr) return;
	// PutWeaponOnBack(*inv.Find(1));
}

void AStealthCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("EquipWeapon1",IE_Pressed, this, &AStealthCharacter::EquipPrimary);
	PlayerInputComponent->BindAction("EquipWeapon2", IE_Pressed, this, &AStealthCharacter::EquipSecondary);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStealthCharacter::LMB_Released);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &AStealthCharacter::ChangeFireMode);
}
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
	isReloading = false;
	
	Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Player Arms"));
	PlayerWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Player Weapon"));
	Arms->SetupAttachment(Camera);
	
	//PlayerWeapon->AttachToComponent(Arms, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative,true), FName(TEXT("R_GunSocket")));

	CanShoot = true;
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
	
		if (*inv.Find(1) == nullptr && *inv.Find(2) == nullptr)
		{
			inv.Add(1, (AGun*)ActorToInteract);
			ActiveWeapon = (AGun*)ActorToInteract;
			SetupAnims();
		}
		else if (*inv.Find(1) == nullptr)
		{
			inv.Add(1, (AGun*)ActorToInteract);
		}
		else if(*inv.Find(2) == nullptr)
		{
			inv.Add(2, (AGun*)ActorToInteract);
		}
		else
		{
			return;
		}

		ActorToInteract->SetActorHiddenInGame(true);
		ActorToInteract->SetActorTickEnabled(false);
		ActorToInteract->SetActorEnableCollision(false);
		

	}
}


void AStealthCharacter::LMB()
{
	if (!ActiveWeapon) return;
	if (isReloading) return;

	if (!CanShoot) return;

	FHitResult Hit;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * ActiveWeapon->ShootDistance;
	ActiveWeapon->Clicked = true;

	if (ActiveWeapon->WeaponFireMode == FireMode::MODE_Single)
	{
		CanShoot = false;
		ActiveWeapon->FireWeaponSingle(&Hit, Start, End, PlayerWeapon, Arms);
		FTimerHandle ShootCooldown;
		GetWorldTimerManager().SetTimer(ShootCooldown, this, &AStealthCharacter::ChangeShoot, ActiveWeapon->FireDelay);
		if (Hit.GetActor() != nullptr)
		{
			if (Hit.GetActor()->ActorHasTag("Player"))
			{
				APlayerCharacter* HitPlayer = (APlayerCharacter*)Hit.GetActor();
				HitPlayer->HealthComponent->TakeDamage(ActiveWeapon->BulletDamage);
			}
		}
	}
	else if (ActiveWeapon->WeaponFireMode == FireMode::MODE_Auto)
	{
		CanShoot = false;
		FireWeaponOnAuto(&Hit,Start,End);
	}

}



void AStealthCharacter::FireWeaponOnAuto(FHitResult *Hit, FVector Start, FVector End)
{
	if (ActiveWeapon->Clicked == false)
	{
		FTimerHandle ShootCooldown;
		GetWorldTimerManager().SetTimer(ShootCooldown, this, &AStealthCharacter::ChangeShoot, ActiveWeapon->FireDelay);
		return;
	}

	Start = Camera->GetComponentLocation();
	End = Start + Camera->GetForwardVector() * ActiveWeapon->ShootDistance;

	ActiveWeapon->FireWeaponAuto(Hit, Start, End, PlayerWeapon, Arms);

	FTimerHandle ShootHandle;
	FTimerDelegate ShootDelegate = FTimerDelegate::CreateUObject(this, &AStealthCharacter::FireWeaponOnAuto, Hit, Start, End);
	
	GetWorldTimerManager().SetTimer(ShootHandle, ShootDelegate, ActiveWeapon->RateOfFire, false);
}

void AStealthCharacter::ChangeFireMode()
{
	if (!ActiveWeapon) return;
	if (isReloading) return;

	if (ActiveWeapon->WeaponFireMode == FireMode::MODE_Auto)
	{
		ActiveWeapon->WeaponFireMode = FireMode::MODE_Single;
	}
	else if (ActiveWeapon->WeaponFireMode == FireMode::MODE_Single)
	{
		ActiveWeapon->WeaponFireMode = FireMode::MODE_Auto;
	}
}

void AStealthCharacter::ChangeShoot()
{
	CanShoot = true;
}

void AStealthCharacter::LMB_Released()
{
	if (ActiveWeapon != nullptr)
	{
		ActiveWeapon->Clicked = false;
	}
}

void AStealthCharacter::Reload()
{
	if (!ActiveWeapon || isReloading) return;
	if (ActiveWeapon->CurrentAmmo == ActiveWeapon->MaxAmmo || ActiveWeapon->NumberOfMagazines == 0) return;
	isReloading = true;
	ActiveWeapon->Clicked = false;
	if (ActiveWeapon->ArmsReloadAnimation != nullptr && ActiveWeapon->WeaponReloadAnimation != nullptr)
	{
		PlayerWeapon->GetAnimInstance()->Montage_Play(ActiveWeapon->WeaponReloadAnimation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		Arms->GetAnimInstance()->Montage_Play(ActiveWeapon->ArmsReloadAnimation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
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

	ActiveWeapon->SetActorHiddenInGame(false);
	ActiveWeapon->SetActorEnableCollision(true);
	ActiveWeapon->SetActorTickEnabled(true);
		

	ActiveWeapon = nullptr; // Unequipe the gun
}

void AStealthCharacter::EquipPrimary()
{
	if (*inv.Find(1) == nullptr)
	{
		print("No primary weapon");
		return;
	}
	ActiveWeapon = *inv.Find(1);
	UE_LOG(LogTemp, Warning, TEXT("Weapon 1"));
}

void AStealthCharacter::EquipSecondary()
{
	if (*inv.Find(2) == nullptr)
	{
		print("No secondary weapon");
		return;
	}
	ActiveWeapon = *inv.Find(2);
	UE_LOG(LogTemp, Warning, TEXT("Weapon 2"));

	
}
void AStealthCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("EquipWeapon1",IE_Pressed, this, &AStealthCharacter::EquipPrimary);
	PlayerInputComponent->BindAction("EquipWeapon2", IE_Pressed, this, &AStealthCharacter::EquipSecondary);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AStealthCharacter::LMB_Released);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &AStealthCharacter::ChangeFireMode);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GunCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Gun Collision"));
	GunCollission->InitBoxExtent(FVector(30, 30, 30));

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetRelativeLocation(FVector(0, 0, 0));
	GunMesh->SetupAttachment(Root);
	GunCollission->SetupAttachment(GunMesh);
	
	WeaponFireMode = FireMode::MODE_Single;
	Clicked = false;
	
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGun::FireWeaponSingle(FHitResult *Hit, FVector Start, FVector End, USkeletalMeshComponent * PlayerWeapon, USkeletalMeshComponent* PlayerArms)
{
	if (CurrentAmmo <= 0) return;

	bool hasHit = GetWorld()->LineTraceSingleByChannel(*Hit, Start, End, ECollisionChannel::ECC_Visibility);
	if (hasHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit->Location, FColor::Green, false, 2, false);
	}
	CurrentAmmo--;
	
	// Animations
	if (ArmsFire01Animation == nullptr || ArmsFire02Animation == nullptr || WeaponFire01Animation == nullptr || WeaponFire02Animation == nullptr) return;

	if (FMath::RandRange(1, 2) == 1)
	{
		PlayerWeapon->GetAnimInstance()->Montage_Play(WeaponFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
	else
	{
		PlayerWeapon->GetAnimInstance()->Montage_Play(WeaponFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
}

void AGun::FireWeaponAuto(FHitResult *Hit, FVector Start, FVector End, USkeletalMeshComponent* PlayerWeapon, USkeletalMeshComponent* PlayerArms)
{
	if (CurrentAmmo <= 0) return;
	if (Clicked == false) return;

	bool hasHit = GetWorld()->LineTraceSingleByChannel(*Hit, Start, End, ECollisionChannel::ECC_Visibility);
	if (hasHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit->Location, FColor::Green, false, 2, false);
	} 
	else
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2, false);
	}
	CurrentAmmo--;

	// Animations
	if (ArmsFire01Animation == nullptr || ArmsFire02Animation == nullptr || WeaponFire01Animation == nullptr || WeaponFire02Animation == nullptr) return;

	if (FMath::RandRange(1, 2) == 1)
	{
		PlayerWeapon->GetAnimInstance()->Montage_Play(WeaponFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
	else
	{
		PlayerWeapon->GetAnimInstance()->Montage_Play(WeaponFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
}


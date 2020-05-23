// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GunCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Gun Collision"));
	GunCollission->InitBoxExtent(FVector(30, 30, 30));

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetRelativeLocation(FVector(0, 0, 0));
	GunMesh->SetupAttachment(Root);
	GunCollission->SetupAttachment(GunMesh);
	
	WeaponFireMode = FireMode::MODE_Single;
	Clicked = false;
	CanClick = true;
	isFiring = false;
	isReloading = false;
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

void AGun::UpdateCanClick()
{
	isFiring = false;
	CanClick = true;
}

void AGun::FireWeapon(FHitResult *Hit, UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms)
{
	if (!CanClick) return;
	Clicked = true;
	isFiring = true;
	if (WeaponFireMode == FireMode::MODE_Single)
	{
		CanClick = false;
		FireWeaponSingle(Hit,Camera,PlayerArms);
		FTimerHandle CanClickHandle;
		GetWorldTimerManager().SetTimer(CanClickHandle, this, &AGun::UpdateCanClick, FireDelay);
	}
	else if (WeaponFireMode == FireMode::MODE_Auto)
	{
		CanClick = false;
		FireWeaponAuto(Hit,Camera,PlayerArms);
	}
	else if (WeaponFireMode == FireMode::MODE_Burst)
	{

	}
}

void AGun::FireWeaponSingle(FHitResult *Hit, UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms)
{
	if (CurrentAmmo <= 0) return;

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * ShootDistance;

	// Action
	bool hasHit = GetWorld()->LineTraceSingleByChannel(*Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel1);
	if (hasHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit->Location, FColor::Green, false, 2, false);
	}
	CurrentAmmo--;
	
	// Animations
	if (ArmsFire01Animation == nullptr || ArmsFire02Animation == nullptr || WeaponFire01Animation == nullptr || WeaponFire02Animation == nullptr) return;

	if (FMath::RandRange(1, 2) == 1)
	{
		GunMesh->GetAnimInstance()->Montage_Play(WeaponFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
	else
	{
		GunMesh->GetAnimInstance()->Montage_Play(WeaponFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
}

void AGun::FireWeaponAuto(FHitResult* Hit, UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms)
{
	
	if (Clicked == false || CurrentAmmo <= 0)
	{
		FTimerHandle CanClickHandle;
		GetWorldTimerManager().SetTimer(CanClickHandle, this, &AGun::UpdateCanClick, FireDelay);
		return;
	}
	if (!Hit) return;
	if (!Camera) return;
	
	// Action
	FVector Start = Camera->GetComponentLocation();
	FVector Direction = Camera->GetForwardVector() * ShootDistance;
	FVector End = Start + Direction;

	FCollisionQueryParams CollisionParams;
	bool hasHit = GetWorld()->LineTraceSingleByChannel(*Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);

	if (hasHit)
	{

		UE_LOG(LogTemp, Warning, TEXT("Object Hit Is: %s"), *(Hit->GetActor()->GetName()));
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

		GunMesh->GetAnimInstance()->Montage_Play(WeaponFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire01Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}
	else
	{
		GunMesh->GetAnimInstance()->Montage_Play(WeaponFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsFire02Animation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}

	// Recursive Call
	FTimerHandle ShootHandle;
	FTimerDelegate ShootDelegate = FTimerDelegate::CreateUObject(this, &AGun::FireWeaponAuto, Hit, Camera, PlayerArms);

	GetWorldTimerManager().SetTimer(ShootHandle, ShootDelegate, RateOfFire, false);
}

void AGun::StopFire()
{
	Clicked = false;
}

void AGun::ReloadWeapon(USkeletalMeshComponent * PlayerArms)
{
	if (CurrentAmmo == MaxAmmo || NumberOfMagazines == 0 || isReloading) return;

	isReloading = true;
	Clicked = false;

	if (ArmsReloadAnimation != nullptr && WeaponReloadAnimation != nullptr)
	{
		GunMesh->GetAnimInstance()->Montage_Play(WeaponReloadAnimation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
		PlayerArms->GetAnimInstance()->Montage_Play(ArmsReloadAnimation, 1.1f, EMontagePlayReturnType::MontageLength, 0, true);
	}

	FTimerHandle ReloadHandle;
	GetWorldTimerManager().SetTimer(ReloadHandle, this, &AGun::UpdateAmmo,ReloadTime);
}

void AGun::UpdateAmmo()
{
	CurrentAmmo = MaxAmmo;
	NumberOfMagazines--;
	isReloading = false;
}

void AGun::ChangeFireMode()
{
	if (isReloading) return;
	if (WeaponFireMode == FireMode::MODE_Auto)
	{
		WeaponFireMode = FireMode::MODE_Single;
	}
	else if (WeaponFireMode == FireMode::MODE_Single)
	{
		WeaponFireMode = FireMode::MODE_Auto;
	}
}
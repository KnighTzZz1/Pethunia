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
#include "PlayerHealthComponent.h"
#include "Pethunia.h"
#include "StealthCharacter.h"



// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GunCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Gun Collision"));
	GunCollission->InitBoxExtent(FVector(30, 30, 30));

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetRelativeLocation(FVector(0, 0, 0));
	GunMesh->SetupAttachment(Root);
	GunCollission->SetupAttachment(GunMesh);

	GunCollission->OnComponentBeginOverlap.AddDynamic(this, &AGun::OnOverlapBegin);
	
	WeaponFireMode = FireMode::MODE_Single;
	Clicked = false;
	CanClick = true;
	isFiring = false;
	isReloading = false;
	GunOwner = false;
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();

	FOnTimelineFloat GunFloatingFunction;

	GunFloatingFunction.BindUFunction(this, FName("HandleGunFloatingProgress"));
	
	IdleTimeline.AddInterpFloat(GunFloatingCurve, GunFloatingFunction);
	IdleTimeline.SetLooping(true);

	UpdateGunPosition();

	IdleTimeline.PlayFromStart();


	FOnTimelineFloat GunShootFunction;
	GunShootFunction.BindUFunction(this, FName("HandleGunShootProgress"));

	GunShakeTimeline.AddInterpFloat(GunFireCurve, GunShootFunction);
	GunShakeTimeline.SetLooping(false);

	

}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!GunOwner)
	{
		
		IdleTimeline.TickTimeline(DeltaTime);
	}
	GunShakeTimeline.TickTimeline(DeltaTime);
}

void AGun::HandleGunFloatingProgress(float value)
{
	FVector NewLocation = FMath::Lerp(InitialLocation, TargetLocation, value);
	SetActorLocation(NewLocation);
}

void AGun::UpdateCanClick()
{
	isFiring = false;
	CanClick = true;
}

void AGun::FireWeapon(USkeletalMeshComponent* PlayerArms)
{
	if (!CanClick) return;
	Clicked = true;
	isFiring = true;
	if (WeaponFireMode == FireMode::MODE_Single)
	{
		CanClick = false;
		FireWeaponSingle(PlayerArms);
		FTimerHandle CanClickHandle;
		GetWorldTimerManager().SetTimer(CanClickHandle, this, &AGun::UpdateCanClick, FireDelay);
	}
	else if (WeaponFireMode == FireMode::MODE_Auto)
	{
		CanClick = false;
		FireWeaponAuto(PlayerArms);
	}
	else if (WeaponFireMode == FireMode::MODE_Burst)
	{

	}
}

void AGun::FireWeaponSingle(USkeletalMeshComponent* PlayerArms)
{
	if (CurrentAmmo <= 0) return;

	FVector Start = Owner_Camera->GetComponentLocation();
	FVector End = Start + Owner_Camera->GetForwardVector() * ShootDistance;
	
	float xOffset = FMath::RandRange(BulletSpreadRadius * (-1), BulletSpreadRadius);
	float zOffset = FMath::RandRange(BulletSpreadRadius * (-1), BulletSpreadRadius);

	End = FVector(End.X + xOffset, End.Y, End.Z + zOffset);

	FHitResult Hit;

	// Action
	bool hasHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel1);
	if (hasHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 2, false);

		if (Hit.GetActor()->ActorHasTag(FName(TEXT("Damagable"))))
		{
			UPlayerHealthComponent* HitComponent = Cast<UPlayerHealthComponent>(Hit.GetActor()->GetComponentByClass(UPlayerHealthComponent::StaticClass()));
			if (HitComponent)
			{
				DrawDebugString(GetWorld(), Hit.ImpactPoint, FString::SanitizeFloat(BulletDamage), nullptr, FColor::White, 0.5f, true);
				HitComponent->TakeDamage(BulletDamage);
			}
		}
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
	
	GunShakeTimeline.PlayFromStart();
}

void AGun::FireWeaponAuto(USkeletalMeshComponent* PlayerArms)
{
	
	if (Clicked == false || CurrentAmmo <= 0)
	{
		FTimerHandle CanClickHandle;
		GetWorldTimerManager().SetTimer(CanClickHandle, this, &AGun::UpdateCanClick, FireDelay);
		return;
	}
	
	if (!Owner_Camera) return;
	
	// Action
	FVector Start = Owner_Camera->GetComponentLocation();
	FVector Direction = Owner_Camera->GetForwardVector() * ShootDistance;
	FVector End = Start + Direction;

	float xOffset = FMath::RandRange(BulletSpreadRadius * (-1), BulletSpreadRadius);
	float zOffset = FMath::RandRange(BulletSpreadRadius * (-1), BulletSpreadRadius);

	End = FVector(End.X + xOffset, End.Y, End.Z + zOffset);

	FHitResult Hit;

	FCollisionQueryParams CollisionParams;
	bool hasHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_GameTraceChannel1, CollisionParams);

	if (hasHit)
	{
		DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Green, false, 2, false);
		UPlayerHealthComponent* HitComponent = Cast<UPlayerHealthComponent>(Hit.GetActor()->GetComponentByClass(UPlayerHealthComponent::StaticClass()));
		if (HitComponent)
		{
			DrawDebugString(GetWorld(), Hit.ImpactPoint, FString::SanitizeFloat(BulletDamage), nullptr, FColor::White, 0.5f, true);
			HitComponent->TakeDamage(BulletDamage);
		}
		
		
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

	GunShakeTimeline.PlayFromStart();

	// Recursive Call
	FTimerHandle ShootHandle;
	FTimerDelegate ShootDelegate = FTimerDelegate::CreateUObject(this, &AGun::FireWeaponAuto, PlayerArms);

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

void AGun::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor && OtherComp && OtherComp->ComponentHasTag(FName(TEXT("GunCheck"))))
	{
		AStealthCharacter* player = (AStealthCharacter*)OtherActor;
		player->TryPickingUpWeapon(this);
	}
}

void AGun::UpdateGunPosition()
{
	InitialLocation = GetActorLocation();
	TargetLocation = InitialLocation + FVector(0, 0, IdleOffset);
}

void AGun::HandleGunShootProgress(float value)
{
	FRotator NewRotation = FRotator(value * HipFire, 0, 0);
	if (Owner_Camera)
	{
		Owner_Camera->SetRelativeRotation(NewRotation);
	}
}

void AGun::UpdateGunRecoil()
{

}
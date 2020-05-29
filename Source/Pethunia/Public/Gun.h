// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ammo.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

#include "Gun.generated.h"


class USkeletalMeshComponent;
class UBoxComponent;
class USceneComponent;
class UAnimMontage;
class UCameraComponent;


UENUM(BlueprintType)
enum class FireMode : uint8
{
	MODE_Single		UMETA(DisplayName = "Single Fire"),
	MODE_Burst		UMETA(DisplayName = "Burst Fire"),
	MODE_Auto		UMETA(DisplayName = "Auto Fire")
};

UCLASS()
class PETHUNIA_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGun();

	UPROPERTY()
		USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
		USkeletalMeshComponent* GunMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
		UBoxComponent* GunCollission;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0", ClampMax = "50"))
		int MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
		int CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0", ClampMax = "10"))
		int NumberOfMagazines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "50000"))
		float ShootDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "10.0"))
		float ReloadTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "10.0"))
		float RateOfFire;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "10.0"))
		float FireDelay;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "100.0"))
		float BulletDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0.0", ClampMax = "300.0"))
		float BulletSpreadRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
		FName GunName;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* ArmsReloadAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* WeaponReloadAnimation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* ArmsFire01Animation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* WeaponFire01Animation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* ArmsFire02Animation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Animations")
		UAnimMontage* WeaponFire02Animation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
		FireMode WeaponFireMode;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
		EAmmoType AmmoType;
	
	void FireWeaponSingle(UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms);
	void FireWeaponBurst();
	void FireWeaponAuto(UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms);
	void FireWeapon(UCameraComponent* Camera, USkeletalMeshComponent* PlayerArms);
	void StopFire();

	void ReloadWeapon(USkeletalMeshComponent* PlayerArms);
	void ChangeFireMode();

	// States
	bool isReloading;
	bool isFiring;

	AActor* GunOwner;
	
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// For Idle Animations
	FTimeline IdleTimeline;
	
	UPROPERTY(EditAnywhere)
		float IdleOffset;
	UPROPERTY(EditAnywhere)
		UCurveFloat* GunFloatingCurve;
	UFUNCTION()
		void HandleGunFloatingProgress(float value);

	void UpdateGunPosition();

private:
	FVector InitialLocation;
	FVector TargetLocation;

	bool Clicked;
	bool CanClick;
	void UpdateCanClick();
	void UpdateAmmo();
	
};

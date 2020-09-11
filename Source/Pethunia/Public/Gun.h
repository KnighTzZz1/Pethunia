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


USTRUCT()
struct FDropLocation
{
	GENERATED_BODY();

public:
	UPROPERTY()
		FVector_NetQuantize endLocation;
};


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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Base")
		USkeletalMeshComponent* PlayerArms;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
		UBoxComponent* GunCollission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun", meta = (ClampMin = "0", ClampMax = "50"))
		int MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Gun")
		int CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Gun", meta = (ClampMin = "0", ClampMax = "10"))
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

	UPROPERTY(BlueprintReadOnly)
		AActor* GunOwner;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
		float HipFire;
	UCameraComponent* Owner_Camera;

	UPROPERTY(ReplicatedUsing = OnRep_Drop)
		FDropLocation DropLocation;

	FTimeline GunShakeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCurveFloat* GunFireCurve;

	FTimerHandle ShootHandle;

	// For Idle Animations
	FTimeline IdleTimeline;

	UPROPERTY(EditAnywhere)
		float IdleOffset;
	UPROPERTY(EditAnywhere)
		UCurveFloat* GunFloatingCurve;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
		void PlayShootingAnimations();

	void FireWeaponSingle();
	void FireWeaponBurst();

	UFUNCTION(Server, reliable)
		void Server_ProccessWeaponAutoFire();

	UFUNCTION(Server, reliable)
		void Server_ProccessWeaponSingleFire();


	void FireWeaponAuto();
	
	void FireWeapon();
	void StopFire();
	
	UFUNCTION(Server, reliable)
		void Server_StopFire();

	void ReloadWeapon(USkeletalMeshComponent* PlayerArms);
	void ChangeFireMode();

	// States
	bool isReloading;
	bool isFiring;


	
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION()
		void HandleGunFloatingProgress(float value);

	void UpdateGunPosition();

	UFUNCTION()
		void HandleGunShootProgress(float value);

	void UpdateGunRecoil();

	UFUNCTION()
		void OnRep_Drop();

	UFUNCTION()
		void RemoveOwnership();
	UFUNCTION()
		void UpdateGunDropLocation(FVector value);



	UPROPERTY(BlueprintReadOnly, Replicated)
		bool CanClick;


	
	bool Clicked;

	UFUNCTION(Server, Reliable)
		void UpdateCanClick();

private:
	FRotator Camera_InitialRotation;
	FRotator Camera_TargetRotation;
	
	FVector InitialLocation;
	FVector TargetLocation;

	
	
	void UpdateAmmo();
	
};

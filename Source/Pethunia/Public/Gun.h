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
	
	void FireWeaponSingle(USkeletalMeshComponent* PlayerArms);
	void FireWeaponBurst();

	UFUNCTION(Server, reliable)
		void Server_ProccessWeaponFire();



	void FireWeaponAuto();

	void ShootBullet(USkeletalMeshComponent* PlayerArms);
	
	void FireWeapon(USkeletalMeshComponent* PlayerArms);
	void StopFire();

	void ReloadWeapon(USkeletalMeshComponent* PlayerArms);
	void ChangeFireMode();

	// States
	bool isReloading;
	bool isFiring;

	UPROPERTY(BlueprintReadOnly)
		AActor* GunOwner;
	
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FTimerHandle ShootHandle;

	// For Idle Animations
	FTimeline IdleTimeline;
	
	UPROPERTY(EditAnywhere)
		float IdleOffset;
	UPROPERTY(EditAnywhere)
		UCurveFloat* GunFloatingCurve;
	UFUNCTION()
		void HandleGunFloatingProgress(float value);

	void UpdateGunPosition();

	// CameraShakes For Shooting

	FTimeline GunShakeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCurveFloat* GunFireCurve;
	
	UFUNCTION()
		void HandleGunShootProgress(float value);
	

	void UpdateGunRecoil();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
		float HipFire;
	UCameraComponent* Owner_Camera;

	UPROPERTY(ReplicatedUsing = OnRep_Drop)
		FDropLocation DropLocation;

	UFUNCTION()
		void OnRep_Drop();

	UFUNCTION()
		void RemoveOwnership();
	UFUNCTION()
		void UpdateGunDropLocation(FVector value);



private:
	FRotator Camera_InitialRotation;
	FRotator Camera_TargetRotation;
	
	FVector InitialLocation;
	FVector TargetLocation;

	bool Clicked;
	bool CanClick;
	void UpdateCanClick();
	void UpdateAmmo();
	
};

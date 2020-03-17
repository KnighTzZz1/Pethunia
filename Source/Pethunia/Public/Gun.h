// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class USceneComponent;
class UAnimMontage;

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


};

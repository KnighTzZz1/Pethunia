// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Ammo.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	TAmmo_M4	UMETA(DisplayName = "M4"),
	TAmmo_AK	UMETA(DisplayName = "AK")
};

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class PETHUNIA_API AAmmo : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmo();

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* AmmoTrigger;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AmmoMesh;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* AmmoScene;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void OverlapBeginEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
		EAmmoType ammoType;
private:
};

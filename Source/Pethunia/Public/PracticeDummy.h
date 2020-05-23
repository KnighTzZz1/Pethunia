// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PracticeDummy.generated.h"

class UStaticMeshComponent;
class UPlayerHealthComponent;

UCLASS()
class PETHUNIA_API APracticeDummy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APracticeDummy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* DummyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* DummyRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UPlayerHealthComponent* HealthComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Meta = (MakeEditWidget = true))
		TArray<FVector> Positions;
	UPROPERTY(EditAnywhere)
		float MovementSpeed;
	UPROPERTY(EditAnywhere)
		float Offset;
private:
	int dif;
	int index;

	FVector TargetLocation;
	FVector Direction;

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "PracticeDummy.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerHealthComponent.h"

// Sets default values
APracticeDummy::APracticeDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy Scene"));
	DummyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dummy Mesh"));

	HealthComponent = CreateDefaultSubobject<UPlayerHealthComponent>(TEXT("Dummy Health"));

	RootComponent = DummyRoot;
	DummyMesh->SetupAttachment(RootComponent);

	index = 0;
	dif = 1;
}

// Called when the game starts or when spawned
void APracticeDummy::BeginPlay()
{
	Super::BeginPlay();
	
	if (Positions.Num() != 0)
	{
		TargetLocation = Positions[0] + GetActorLocation();
		Direction = (TargetLocation - DummyMesh->GetComponentLocation()).GetSafeNormal();
	}

}

// Called every frame
void APracticeDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Positions.Num() != 0)
	{
		
		FVector ObjectLocation = DummyMesh->GetComponentLocation();
		
		DummyMesh->AddRelativeLocation(Direction * MovementSpeed);


		if (FVector::Dist(TargetLocation, DummyMesh->GetComponentLocation()) <= Offset)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mivedi Objectze"));
			if (index == Positions.Num() - 1)
				dif = -1;
			else if (index == 0)
				dif = 1;

			index += dif;
			TargetLocation = Positions[index] + GetActorLocation();
			Direction = (TargetLocation - ObjectLocation).GetSafeNormal();
		}
	}

}


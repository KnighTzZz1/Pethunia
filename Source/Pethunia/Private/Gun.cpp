// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GunCollission = CreateDefaultSubobject<UBoxComponent>(TEXT("Gun Collision"));
	GunCollission->InitBoxExtent(FVector(30, 30, 30));

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetRelativeLocation(FVector(0, 0, 0));
	GunMesh->SetupAttachment(RootComponent);
	GunCollission->SetupAttachment(GunMesh);
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


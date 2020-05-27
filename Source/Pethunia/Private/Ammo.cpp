// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Gun.h"
#include "Components/StaticMeshComponent.h"
#include "StealthCharacter.h"
#include "Components/BoxComponent.h"



// Sets default values
AAmmo::AAmmo()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	AmmoTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Ammo Trigger"));
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ammo Mesh"));
	AmmoScene = CreateDefaultSubobject<USceneComponent>(TEXT("Ammo Scene"));

	RootComponent = AmmoScene;

	AmmoMesh->SetupAttachment(AmmoScene);
	AmmoTrigger->SetupAttachment(AmmoMesh);
	
	AmmoTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::OverlapBeginEvent);

}

// Called when the game starts or when spawned
void AAmmo::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAmmo::OverlapBeginEvent(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		
		AStealthCharacter* player = (AStealthCharacter*)OtherActor;
		if (!player->ActiveWeapon) return;
		if (player->ActiveWeapon->AmmoType == ammoType)
		{
			player->ActiveWeapon->NumberOfMagazines++;
			Destroy();
		}
	}
}

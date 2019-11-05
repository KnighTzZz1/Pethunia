// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleAI.h"
#include "TimerManager.h"

// Sets default values
ASampleAI::ASampleAI()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bSampleState = SampleStates::idle;
}

// Called when the game starts or when spawned
void ASampleAI::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASampleAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASampleAI::MoveSample()
{
}
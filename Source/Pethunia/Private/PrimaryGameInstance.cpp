// Fill out your copyright notice in the Description page of Project Settings.


#include "PrimaryGameInstance.h"
#include "Engine/Engine.h"

UPrimaryGameInstance::UPrimaryGameInstance(const FObjectInitializer& ObjectInitializer)
{

}
void UPrimaryGameInstance::init()
{

}

void UPrimaryGameInstance::Host()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	World->ServerTravel("/Game/Level/CodeMap?listen");
}

void UPrimaryGameInstance::Join(const FString& Address)
{
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}
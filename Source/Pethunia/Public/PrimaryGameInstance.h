// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PrimaryGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PETHUNIA_API UPrimaryGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
		UPrimaryGameInstance(const FObjectInitializer& ObjectInitializer);
		virtual void init();

		UFUNCTION(Exec)
			void Host();
		UFUNCTION(Exec)
		void Join(const FString& Address);
};

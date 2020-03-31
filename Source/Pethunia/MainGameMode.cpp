// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

void AMainGameMode::BeginPlay()
{
    
}

void AMainGameMode::CheckIfGameOver()
{
    if(GetNumPlayers() == 1)
    {
        GameOver();
    }
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameInstance.h"

#include "Engine.h"

void UCSGameInstance::LoadMainMenu()
{
    if (MainMenuMap.IsEmpty())
        return;

    GetWorld()->ServerTravel(MainMenuMap);
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API UCSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

    /** Server travel to main menu */
    UFUNCTION(BlueprintCallable, Category = "Travel")
    void LoadMainMenu();

protected:

    UPROPERTY(EditDefaultsOnly, Category = "Maps")
    FString MainMenuMap;
};

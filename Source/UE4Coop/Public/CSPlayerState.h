// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void AddScore(float ScoreAmount);
};

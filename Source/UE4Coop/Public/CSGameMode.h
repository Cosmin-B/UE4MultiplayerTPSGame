// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

    UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
    void SpawnNewBot();

    void SpawnBotTimerElapse();

    void StartWave();

    void EndWave();

    void PrepareForNextWave();

    void CheckWaveState();

protected:

    int WaveCount;

    int NumberOfBotsToSpawn;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float TimeBetweenWaves;

    FTimerHandle TimerHandle_BotSpawner;

    FTimerHandle TimerHandle_NextWaveStart;

public:

    ACSGameMode();

    virtual void StartPlay() override;
};

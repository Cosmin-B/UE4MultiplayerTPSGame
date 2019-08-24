// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/CSGameMode.h"
#include "CSWaveGameMode.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSWaveGameMode : public ACSGameMode
{
	GENERATED_BODY()
	
protected:
        
    /** Begin ACSGameMode Interface */
    virtual void HandleRoundIsStarting() override;
    virtual void HandleRoundHasStarted() override;
    virtual void HandleRoundHasEnded() override;
    virtual bool ReadyToEndRound_Implementation() override;
    virtual bool ReadyToEndMatch_Implementation() override;
    /** End ACSGameMode Interface */

    /** Spawn a new bot from blueprint */
    UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")  
    void SpawnNewBot();

    /** Update Number of Bots to Spawn, and call SpawnNewBot */
    void SpawnBotTimerElapse();

private:

    /** Initial Number of bots to spawn, set at the beginning of each round */
    int NumberOfBotsToSpawn;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float TimeBetweenWaves;

    /** Timer Handle for efficient management of BotSpawning */
    FTimerHandle TimerHandle_BotSpawner;
};

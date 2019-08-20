// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CSGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, Victim, AActor*, Killer, AController*, KillerController);

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

    void GameOver();

    void PrepareForNextWave();

    void CheckWaveState();

    void CheckAnyPlayerAlive();

    void SetWaveState(EWaveState NewState);

    void RespawnDeadPlayers();

protected:

    int WaveCount;

    int NumberOfBotsToSpawn;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float TimeBetweenWaves;

    FTimerHandle TimerHandle_BotSpawner;

    FTimerHandle TimerHandle_NextWaveStart;

    FTimerHandle TimerHandle_CheckWaveState;

    FTimerHandle TimerHandle_CheckAnyPlayerAlive;

    /** Flag to indicate if this Game Mode allows friendly fire */
    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    bool bAllowFriendlyFire;

public:

    ACSGameMode();

    virtual void StartPlay() override;

    /** Whether or not this Game Mode allows friendly fire */
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    bool IsFriendlyFireAllowed();

    UPROPERTY(BlueprintAssignable, Category = "GameMode")
    FOnActorKilled OnActorKilled;
};

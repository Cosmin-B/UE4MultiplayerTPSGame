// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameMode.h"
#include "Components/CSHealthComponent.h"
#include "TimerManager.h"


ACSGameMode::ACSGameMode()
{
    TimeBetweenWaves = 5.0f;
}

void ACSGameMode::StartPlay()
{
    Super::StartPlay();

    PrepareForNextWave();

    FTimerHandle TimerHandle_CheckWaveState;
    GetWorldTimerManager().SetTimer(TimerHandle_CheckWaveState, this, &ACSGameMode::CheckWaveState, 1.0f, true, 0.0f);
}

void ACSGameMode::StartWave()
{
    WaveCount++;

    NumberOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ACSGameMode::SpawnBotTimerElapse, 2.0f, true, 0.0f);
}

void ACSGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ACSGameMode::PrepareForNextWave()
{

    GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ACSGameMode::StartWave, TimeBetweenWaves);
}

void ACSGameMode::CheckWaveState()
{
    bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

    if (NumberOfBotsToSpawn > 0 || bIsPreparingForNextWave)
        return;

    bool bIsAnyBotAlive = false;

    for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
    {
        APawn* Pawn = Iterator->Get();
        if(Pawn == nullptr || Pawn->IsPlayerControlled())
            continue;

        UCSHealthComponent* HealthComp = Cast<UCSHealthComponent>(Pawn->GetComponentByClass(UCSHealthComponent::StaticClass()));

        if(HealthComp == nullptr)
            continue;

        if (HealthComp->GetHealth() > 0.0f)
        {
            bIsAnyBotAlive = true;
            break;
        }
    }

    if (!bIsAnyBotAlive)
        PrepareForNextWave();
}

void ACSGameMode::SpawnBotTimerElapse()
{
    SpawnNewBot();

    NumberOfBotsToSpawn--;

    if (NumberOfBotsToSpawn <= 0)
        EndWave();
}

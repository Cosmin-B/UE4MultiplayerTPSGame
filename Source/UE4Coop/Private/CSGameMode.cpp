// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameMode.h"
#include "CSGameState.h"
#include "CSPlayerState.h"
#include "Components/CSHealthComponent.h"
#include "TimerManager.h"


ACSGameMode::ACSGameMode()
{
    TimeBetweenWaves = 5.0f;

    GameStateClass = ACSGameState::StaticClass();
    PlayerStateClass = ACSPlayerState::StaticClass();
}

void ACSGameMode::StartPlay()
{
    Super::StartPlay();

    PrepareForNextWave();

    GetWorldTimerManager().SetTimer(TimerHandle_CheckWaveState, this, &ACSGameMode::CheckWaveState, 1.0f, true, 0.0f);
    GetWorldTimerManager().SetTimer(TimerHandle_CheckAnyPlayerAlive, this, &ACSGameMode::CheckAnyPlayerAlive, 0.9f, true);
}

void ACSGameMode::StartWave()
{
    WaveCount++;

    NumberOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ACSGameMode::SpawnBotTimerElapse, 2.0f, true, 0.0f);

    SetWaveState(EWaveState::WaveInProgress);
}

void ACSGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

    SetWaveState(EWaveState::WaitingToComplete);
}

void ACSGameMode::GameOver()
{
    EndWave();

    GetWorldTimerManager().ClearTimer(TimerHandle_CheckWaveState);
    GetWorldTimerManager().ClearTimer(TimerHandle_CheckAnyPlayerAlive);

    SetWaveState(EWaveState::GameOver);
}

void ACSGameMode::PrepareForNextWave()
{
    GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ACSGameMode::StartWave, TimeBetweenWaves);

    SetWaveState(EWaveState::WaitingToStart);

    RespawnDeadPlayers();
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
    {
        SetWaveState(EWaveState::WaveComplete);

        PrepareForNextWave();
    }
}

void ACSGameMode::CheckAnyPlayerAlive()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();

        if (PC == nullptr)
            continue;

        APawn * PlayerPawn = PC->GetPawn();

        if (PlayerPawn == nullptr)
            continue;

        UCSHealthComponent* HealthComp = 
            Cast<UCSHealthComponent>(PlayerPawn->GetComponentByClass(UCSHealthComponent::StaticClass()));

        if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
            return;
    }

    GameOver();
}

void ACSGameMode::SetWaveState(EWaveState NewState)
{
    ACSGameState* GS = GetGameState<ACSGameState>();

    if (ensureAlways(GS))
        GS->SetWaveState(NewState);
}

void ACSGameMode::RespawnDeadPlayers()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();

        if (PC == nullptr)
            continue;

        APawn * PlayerPawn = PC->GetPawn();

        if (PlayerPawn == nullptr)
            RestartPlayer(PC);
    }
}

void ACSGameMode::SpawnBotTimerElapse()
{
    SpawnNewBot();

    NumberOfBotsToSpawn--;

    if (NumberOfBotsToSpawn <= 0)
        EndWave();
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWaveGameMode.h"
#include "CSHealthComponent.h"

//////////////////////////////////////////////////////////////////////////
// Wave System (round based)

void ACSWaveGameMode::HandleRoundIsStarting()
{
    Super::HandleRoundIsStarting();

    NumberOfBotsToSpawn = 2 * GetCurrentRound();
}

void ACSWaveGameMode::HandleRoundHasStarted()
{
    Super::HandleRoundHasStarted();

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ACSWaveGameMode::SpawnBotTimerElapse, 2.0f, true, 0.0f);
}

void ACSWaveGameMode::HandleRoundHasEnded()
{
    Super::HandleRoundHasEnded();

    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

bool ACSWaveGameMode::ReadyToEndRound_Implementation()
{
    // If there are no remaining bots alive the end the round
    if (NumberOfBotsToSpawn > 0)
        return false;

    bool bIsAnyBotAlive = false;

    for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
    {
        APawn* Pawn = Iterator->Get();
        if (Pawn == nullptr || Pawn->IsPlayerControlled())
            continue;

        UCSHealthComponent* HealthComp = Cast<UCSHealthComponent>(Pawn->GetComponentByClass(UCSHealthComponent::StaticClass()));

        if (HealthComp == nullptr)
            continue;

        if (HealthComp->GetHealth() > 0.0f)
        {
            bIsAnyBotAlive = true;
            break;
        }
    }

    return !bIsAnyBotAlive;
}

bool ACSWaveGameMode::ReadyToEndMatch_Implementation()
{
    bool bReadyToEndMatch = GetMatchState() == MatchState::PostRound && GetCurrentRound() >= RoundsToWin;

    if (!bReadyToEndMatch)
    {
        bReadyToEndMatch = true;

        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();

            if (PC == nullptr)
                continue;

            APawn* PlayerPawn = PC->GetPawn();

            if (PlayerPawn == nullptr)
                continue;

            UCSHealthComponent* HealthComp =
                Cast<UCSHealthComponent>(PlayerPawn->GetComponentByClass(UCSHealthComponent::StaticClass()));

            if (ensure(HealthComp) && !HealthComp->IsDead())
            {
                bReadyToEndMatch = false;
                break;
            }
        }
    }

    return bReadyToEndMatch;
}

//////////////////////////////////////////////////////////////////////////
// Bot spawning logic

void ACSWaveGameMode::SpawnBotTimerElapse()
{
    SpawnNewBot();

    NumberOfBotsToSpawn--;

    if (NumberOfBotsToSpawn <= 0)
        GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}
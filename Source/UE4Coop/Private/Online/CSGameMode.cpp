// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameMode.h"
#include "CSGameState.h"
#include "CSCharacter.h"
#include "CSPlayerState.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/GameSession.h"
#include "Components/CSHealthComponent.h"
#include "TimerManager.h"

namespace MatchState
{
    const FName PreRound        = FName(TEXT("PreRound"));
    const FName RoundInProgress = FName(TEXT("RoundInProgress"));
    const FName PostRound       = FName(TEXT("PostRound"));
}

ACSGameMode::ACSGameMode()
{
    GameStateClass = ACSGameState::StaticClass();
    PlayerStateClass = ACSPlayerState::StaticClass();

    bAllowFriendlyFire = true;

    MaxRoundDuration = 100.0f;
    MaxScore = 100.0f;
    TravelDelay = 3.0f;

    RoundsToWin = 2;

    CurrentRound = 0;

    PreRoundDuration = 10.f;
    PostRoundDuration = 5.f;
}

//////////////////////////////////////////////////////////////////////////
// Matching System

bool ACSGameMode::IsMatchInProgress() const
{
    if (MatchState == MatchState::InProgress
        || MatchState == MatchState::PreRound
        || MatchState == MatchState::PostRound
        || MatchState == MatchState::RoundInProgress)
        return true;

    return false;
}

void ACSGameMode::StartMatch()
{
    Super::StartMatch();

    UE_LOG(LogTemp, Warning, TEXT("Start Match"));
}

void ACSGameMode::EndMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("End Match"));

    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

    //GetWorld()->GetTimerManager().SetTimer(LoadLobbyHandle, this, &ADJVGameMode::LoadLobbyMap, TravelDelay, false);

    Super::EndMatch();
}

void ACSGameMode::OnMatchStateSet()
{
    // Copy and override from GameMode.cpp
    FGameModeEvents::OnGameModeMatchStateSetEvent().Broadcast(MatchState);

    // Call change callbacks
    if (MatchState == MatchState::WaitingToStart)
    {
        HandleMatchIsWaitingToStart();
    }
    else if (MatchState == MatchState::InProgress)
    {
        HandleMatchHasStarted();
    }
    else if (MatchState == MatchState::WaitingPostMatch)
    {
        HandleMatchHasEnded();
    }
    else if (MatchState == MatchState::LeavingMap)
    {
        HandleLeavingMap();
    }
    else if (MatchState == MatchState::Aborted)
    {
        HandleMatchAborted();
    }
    else if (MatchState == MatchState::PreRound)
    {
        HandleRoundIsStarting();
    }
    else if (MatchState == MatchState::RoundInProgress)
    {
        HandleRoundHasStarted();
    }
    else if (MatchState == MatchState::PostRound)
    {
        HandleRoundHasEnded();
    }
}

void ACSGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    UE_LOG(LogTemp, Warning, TEXT("Match has started"));
}

bool ACSGameMode::ReadyToStartMatch_Implementation()
{
    return Super::ReadyToStartMatch_Implementation();
}

bool ACSGameMode::ReadyToEndMatch_Implementation()
{
    return false;
}

//////////////////////////////////////////////////////////////////////////
// Round System

void ACSGameMode::StartPreRound()
{
    SetMatchState(MatchState::PreRound);

    CSGameState->SetTimeRemaining(PreRoundDuration);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_TickGameTime, this, &ACSGameMode::TickGameTime, 1.0f, true);
}

void ACSGameMode::StartRound()
{
    if (MatchState != MatchState::PreRound)
        return;

    SetMatchState(MatchState::RoundInProgress);

    CSGameState->SetTimeRemaining(MaxRoundDuration);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_TickGameTime, this, &ACSGameMode::TickGameTime, 1.0f, true);
}

void ACSGameMode::EndRound()
{
    SetMatchState(MatchState::PostRound);

    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_TickGameTime);

    CSGameState->SetTimeRemaining(PostRoundDuration);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_TickGameTime, this, &ACSGameMode::TickGameTime, 1.0f, true);
}

void ACSGameMode::HandleRoundIsStarting()
{
    if (CurrentRound)
        RespawnDeadPlayers();

    CurrentRound++;

    CSGameState->SetCurrentRound(CurrentRound);
}

bool ACSGameMode::ReadyToStartRound_Implementation()
{
    return false;
}

bool ACSGameMode::ReadyToEndRound_Implementation()
{
    // Check either a team or player condition, but we don't have yet such a game mode
    return false;
}

void ACSGameMode::HandleRoundHasEnded()
{
    // In case we have some complicated game mode logic for the end of the round
    // this should happen here. Such as deciding which team/player won the round
}

bool ACSGameMode::ReadyToStartPreRound_Implementation()
{
    // We let the timer start new preround
    return false;
}

//////////////////////////////////////////////////////////////////////////
// Game Logic Handling

void ACSGameMode::InitGameState()
{
    Super::InitGameState();

    CSGameState = Cast<ACSGameState>(GameState);
    if (CSGameState)
    {
        CSGameState->SetTimeRemaining(MaxRoundDuration);

        CSGameState->SetMaxScore(MaxScore);

        CSGameState->SetMaxRounds(RoundsToWin);
    }
}

void ACSGameMode::RespawnDeadPlayers()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();

        if (PC == nullptr)
            continue;

        APawn* PlayerPawn = PC->GetPawn();

        if (PlayerPawn == nullptr)
            RestartPlayer(PC);
    }
}

void ACSGameMode::Tick(float DeltaSeconds)
{
    AInfo::Tick(DeltaSeconds); // we call super from GameModeBase

    // Override stuff from GameMode.cpp
    if (GetMatchState() == MatchState::WaitingToStart)
    {
        // Check to see if we should start the match
        if (ReadyToStartMatch())
        {
            UE_LOG(LogGameMode, Log, TEXT("GameMode returned ReadyToStartMatch"));
            StartMatch();
        }
    }

    if (GetMatchState() == MatchState::InProgress)
    {
        //transition to preround
        UE_LOG(LogGameMode, Log, TEXT("GameMode entered InProgress and is transitioning to PreRound"));
        StartPreRound();
    }

    if (GetMatchState() == MatchState::PreRound)
    {
        // Check to see if we should start the round
        if (ReadyToStartRound())
        {
            UE_LOG(LogGameMode, Log, TEXT("GameMode returned ReadyToStartRound"));
            StartRound();
        }
    }

    if (GetMatchState() == MatchState::RoundInProgress)
    {
        // Check to see if we should end the round
        if (ReadyToEndRound())
        {
            UE_LOG(LogGameMode, Log, TEXT("GameMode returned ReadyToEndRound"));
            EndRound();
        }
    }

    if (GetMatchState() == MatchState::PostRound)
    {
        if (ReadyToEndMatch())
        {
            UE_LOG(LogGameMode, Log, TEXT("GameMode returned ReadyToEndMatch"));
            EndMatch();
        }
        else if (ReadyToStartPreRound())
            StartPreRound();
    }
}

bool ACSGameMode::IsFriendlyFireAllowed()
{
    return bAllowFriendlyFire;
}

void ACSGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
    ACSPlayerState* KillerPlayerState = Killer ? Cast<ACSPlayerState>(Killer->PlayerState) : nullptr;
    ACSPlayerState* VictimPlayerState = KilledPlayer ? Cast<ACSPlayerState>(Killer->PlayerState) : nullptr;

    if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
        KillerPlayerState->ScoreKill(ScorePerKill);

    OnActorKilled.Broadcast(KilledPawn, Killer->GetPawn(), Killer);
}

void ACSGameMode::TickGameTime()
{
    if (CSGameState)
    {
        float TimeRemaining = CSGameState->GetTimeRemaining() - 1;

        CSGameState->SetTimeRemaining(TimeRemaining);

        if (TimeRemaining <= 0)
        {
            GetWorld()->GetTimerManager().ClearTimer(TimerHandle_TickGameTime);

            if (MatchState == MatchState::RoundInProgress)
                EndRound();
            else if (MatchState == MatchState::PreRound)
                StartRound();
            else if (MatchState == MatchState::PostRound)
                StartPreRound();
        }
    }
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameState.h"
#include "CSGameMode.h"

#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// Writing Data

void ACSGameState::SetTimeRemaining(const float& Time)
{
    if (Role == ENetRole::ROLE_Authority)
        TimeRemaining = Time;
}

void ACSGameState::SetMaxScore(const int32& MaximumScore)
{
    if (Role == ENetRole::ROLE_Authority)
        MaxScore = MaximumScore;
}

void ACSGameState::SetMaxRounds(int32 MaxRoundsNum)
{
    if (Role == ENetRole::ROLE_Authority)
        MaxRounds = MaxRoundsNum;
}

void ACSGameState::SetCurrentRound(const int32& Round)
{
    if (Role == ENetRole::ROLE_Authority)
        CurrentRound = Round;
}

void ACSGameState::SetPlayerWinner(bool bPlayerWon)
{
    bPlayerWinner = bPlayerWon;
}

//////////////////////////////////////////////////////////////////////////
// Reading Data

float ACSGameState::GetTimeRemaining() const
{
    return TimeRemaining;
}

int32 ACSGameState::GetMaxScore() const
{
    return MaxScore;
}

int32 ACSGameState::GetMaxRounds() const
{
    return MaxRounds;
}

int32 ACSGameState::GetCurrentRound() const
{
    return CurrentRound;
}

bool ACSGameState::IsInputEnabled() const
{
    return GetMatchState() == MatchState::RoundInProgress || GetMatchState() == MatchState::PostRound || GetMatchState() == MatchState::WaitingPostMatch;
}

bool ACSGameState::IsPlayerWinner() const
{
    return bPlayerWinner;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ACSGameState::OnRep_MatchState()
{
    Super::OnRep_MatchState();

    OnMatchStateChanged.Broadcast(PreviousMatchState, MatchState);
}

void ACSGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSGameState, TimeRemaining);
    DOREPLIFETIME(ACSGameState, MaxScore);
    DOREPLIFETIME(ACSGameState, MaxRounds);
    DOREPLIFETIME(ACSGameState, CurrentRound);
    DOREPLIFETIME(ACSGameState, bPlayerWinner);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPlayerState.h"

#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// APlayerState Interface

void ACSPlayerState::Reset()
{
    Super::Reset();

    Score = 0;
}

//////////////////////////////////////////////////////////////////////////
// Statistics

void ACSPlayerState::AddScore(float ScoreAmount)
{
    Score += ScoreAmount;
}

void ACSPlayerState::ScoreKill(int32 ScoreAmount)
{
    NumberOfKills++;
    AddScore(ScoreAmount);
}

void ACSPlayerState::RegisterShotFired()
{
    NumberOfShotsFired++;
}

void ACSPlayerState::RegisterShotHit()
{
    NumberOfShotsHit++;
}

void ACSPlayerState::RegisterDamageDone(int32 DamageAmount)
{
    DamageDone += DamageAmount;
}

void ACSPlayerState::RegisterDamageTaken(int32 DamageTakenAmount)
{
    DamageTaken += DamageTakenAmount;
}

//////////////////////////////////////////////////////////////////////////
// Read data

float ACSPlayerState::GetScore() const
{
    return Score;
}

float ACSPlayerState::GetKills() const
{
    return NumberOfKills;
}

float ACSPlayerState::GetShotsFired() const
{
    return NumberOfShotsFired;
}

float ACSPlayerState::GetShotsHit() const
{
    return NumberOfShotsHit;
}

float ACSPlayerState::GetDamageDone() const
{
    return DamageDone;
}

float ACSPlayerState::GetDamageTaken() const
{
    return DamageTaken;
}

float ACSPlayerState::CalculateShotAccuracy() const
{
    // Avoid division by 0
    if (NumberOfShotsFired == 0)
        return 0.0f;

    return (NumberOfShotsHit / (float)NumberOfShotsFired) * 100.0f;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ACSPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // We want to replicate this to everyone
    DOREPLIFETIME(ACSPlayerState, NumberOfKills);

    // We want to replicate these only to the owner, as this is the one who is gonna ask for them

    // Shots Accurracy
    DOREPLIFETIME_CONDITION(ACSPlayerState, NumberOfShotsFired, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACSPlayerState, NumberOfShotsHit, COND_OwnerOnly);

    // Damage
    DOREPLIFETIME_CONDITION(ACSPlayerState, DamageDone, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACSPlayerState, DamageTaken, COND_OwnerOnly);
}
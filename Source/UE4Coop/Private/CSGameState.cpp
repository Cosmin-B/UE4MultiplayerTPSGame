// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameState.h"
#include "Net/UnrealNetwork.h"

void ACSGameState::OnRep_WaveState(EWaveState OldState)
{
    WaveStateChanged(WaveState, OldState);
}

void ACSGameState::SetWaveState(EWaveState NewState)
{
    if (Role == ENetRole::ROLE_Authority)
    {
        EWaveState OldState = WaveState;

        WaveState = NewState;

        OnRep_WaveState(OldState);
    }
}

void ACSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSGameState, WaveState);
}
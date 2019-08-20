// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPowerUpBase.h"
#include "CSCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACSPowerUpBase::ACSPowerUpBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    PeriodicTimer = 0;
    TotalNumberOfTicks = 0;
    bIsPowerUpActive = false;

    SetReplicates(true);
}

void ACSPowerUpBase::OnTick()
{
    TicksCounter++;

    OnPowerUpTicked();

    if (TicksCounter >= TotalNumberOfTicks)
    {
        OnExpired();

        bIsPowerUpActive = false;
        OnRep_PowerUpActive();

        // Stop the timer
        GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);

        Target = nullptr;
    }
}

void ACSPowerUpBase::Activate(ACSCharacter* TargetPawn)
{
    Target = TargetPawn;

    OnActivated();

    bIsPowerUpActive = true;
    OnRep_PowerUpActive();

    if (PeriodicTimer)
    {
        GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &ACSPowerUpBase::OnTick, PeriodicTimer, true);
    }
    else
        OnTick();
}

void ACSPowerUpBase::OnRep_PowerUpActive()
{
    OnPowerUpStateChanged(bIsPowerUpActive);
}

void ACSPowerUpBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSPowerUpBase, bIsPowerUpActive);
}
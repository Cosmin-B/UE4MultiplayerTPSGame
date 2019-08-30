// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSPowerUpBase.generated.h"

class ACSCharacter;

UCLASS()
class UE4COOP_API ACSPowerUpBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSPowerUpBase();

protected:

    UFUNCTION()
    void OnTick();

    UFUNCTION()
    void OnRep_PowerUpActive();

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnPowerUpStateChanged(bool bNewIsActive);

    /* Time between power up ticks*/
    UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
    float PeriodicTimer;

    UPROPERTY(EditDefaultsOnly, Category = "PowerUps")
    int TotalNumberOfTicks;

    UPROPERTY(ReplicatedUsing=OnRep_PowerUpActive)
    bool bIsPowerUpActive;

    int TicksCounter;

    UPROPERTY(BlueprintReadOnly, Category = "PowerUps")
    ACSCharacter* Target;

    FTimerHandle TimerHandle_PowerUpTick;
public:	

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnPowerUpTicked();

    UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
    void OnExpired();

    void Activate(ACSCharacter* TargetPawn);
};

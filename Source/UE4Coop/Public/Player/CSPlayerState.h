// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSPlayerState : public APlayerState
{
	GENERATED_BODY()
	

protected:

    /** Begin APlayerState Interface */
    virtual void Reset() override;
    /** End APlayerState Interface */

public:

    //////////////////////////////////////////////////////////////////////////
    // Read data

    /** Get current score */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetScore() const;

    /** Get number of kills */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetKills() const;

    /** Get number of shots fired */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetShotsFired() const;

    /** Get number of shots hit */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetShotsHit() const;

    /** Get total damage done by this player */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetDamageDone() const;

    /** Get total damage taken by this player */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetDamageTaken() const;

    /** Calculate the shot accuracy by using (ShotsHit / ShotsFired) * 100.0f */
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float CalculateShotAccuracy() const;

public:

    //////////////////////////////////////////////////////////////////////////
    // Statistics

    /** Increase player score by X amount */
    void AddScore(float ScoreAmount);

    /** Player killed someone, so score it */
    void ScoreKill(int32 ScoreAmount);

    /** Player fired his weapon */
    void RegisterShotFired();

    /** Player hit something with his weapon */
    void RegisterShotHit();

    /** Player did damage */
    void RegisterDamageDone(int32 DamageAmount);

    /** Player took damage */
    void RegisterDamageTaken(int32 DamageTakenAmount);

private:

    /** Total number of kills */
    UPROPERTY(Transient, Replicated)
    int32 NumberOfKills;

    /** Total number of bullets fired (To calculate Shot Accuracy) */
    UPROPERTY(Transient, Replicated)
    int32 NumberOfShotsFired;

    /** Total number of bullets hit (To calculate Shot Accuracy) */
    UPROPERTY(Transient, Replicated)
    int32 NumberOfShotsHit;

    /** Total amount of damage done */
    UPROPERTY(Transient, Replicated)
    int32 DamageDone;

    /** Total amount of damage taken */
    UPROPERTY(Transient, Replicated)
    int32 DamageTaken;
};

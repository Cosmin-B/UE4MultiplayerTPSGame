// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CSGameMode.generated.h"

namespace MatchState
{
    // copy from GameMode.h
    //extern ENGINE_API const FName EnteringMap;			// We are entering this map, actors are not yet ticking
    //extern ENGINE_API const FName WaitingToStart;		// Actors are ticking, but the match has not yet started
    //extern ENGINE_API const FName InProgress;			// Normal gameplay is occurring. Specific games will have their own state machine inside this state
    //extern ENGINE_API const FName WaitingPostMatch;		// Match has ended so we aren't accepting new players, but actors are still ticking
    //extern ENGINE_API const FName LeavingMap;			// We are transitioning out of the map to another location
    //extern ENGINE_API const FName Aborted;				// Match has failed due to network issues or other problems, cannot continue

    // If a game needs to add additional states, you may need to override HasMatchStarted and HasMatchEnded to deal with the new states
    // Do not add any states before WaitingToStart or after WaitingPostMatch

    // extending functionality for Coop Game
    extern UE4COOP_API const FName PreRound;
    extern UE4COOP_API const FName RoundInProgress; // we avoid InProgress state and use this one instead, InProgress state is used only once at the beginning to run the necessary initialization from GameMode class
    extern UE4COOP_API const FName PostRound;
}

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, Victim, AActor*, Killer, AController*, KillerController);

/**
* Note for MatchState transitions:
*  natural progression: EnteringMap -> WaitingToStart -> InProgress(entered once) ->
*  -> PreRound -> RoundInProgress -> PostRound -> WaitingPostMatch OR PreRound

* We use InProgress state at the beginning to run the necessary initialization from GameMode class then we transition to PreRound
* match can now end only from PostRound state (when ReadyToEndMatch returns true) or by manually ending it
 */
UCLASS()
class UE4COOP_API ACSGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:

    //////////////////////////////////////////////////////////////////////////
    // Matching System

    /** Begin AGameMode Interface */
    virtual void StartMatch() override;
    virtual void EndMatch() override;
    virtual void OnMatchStateSet() override;
    virtual void HandleMatchHasStarted() override;
    virtual bool ReadyToStartMatch_Implementation() override;
    virtual bool ReadyToEndMatch_Implementation() override;
    /** End AGameMode Interface */

    virtual bool IsMatchInProgress() const override;

    //////////////////////////////////////////////////////////////////////////
    // Round System

    /** Transitions from PreRound to RoundInProgress */
    UFUNCTION(BlueprintCallable, Category = "Game")
    virtual void StartPreRound();

    /** Transitions from PreRound to RoundInProgress */
    UFUNCTION(BlueprintCallable, Category = "Game")
    virtual void StartRound();

    /** Transitions to PostRound state */
    UFUNCTION(BlueprintCallable, Category = "Game")
    virtual void EndRound();

    /** Returns true if ready to Start a round. Games should override this */
    UFUNCTION(BlueprintNativeEvent, Category = "Game")
    bool ReadyToStartPreRound();

    /** Called when the state transitions to PreRound.
    * ex.: Should reset scores
    */
    virtual void HandleRoundIsStarting();

    /** Returns true if ready to Start a round. Games should override this */
    UFUNCTION(BlueprintNativeEvent, Category = "Game")
    bool ReadyToStartRound();

    /** Called when the state transitions to RoundInProgress */
    virtual void HandleRoundHasStarted() {};

    /** Returns true if ready to End Round. Games should override this */
    UFUNCTION(BlueprintNativeEvent, Category = "Game")
    bool ReadyToEndRound();

    /** Called when the map transitions to PostRound.
    * ex.: should update round scores
    */
    virtual void HandleRoundHasEnded();

private:

    /** Current Round Number */
    int32 CurrentRound;

public:

    /** Respawn all players */
    virtual void RespawnDeadPlayers();

    // Overriden for rounds functionality
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION()
    void LoadMainMenuMap();

public:

    //////////////////////////////////////////////////////////////////////////
    // Reading Data

    /** Get Current Round */
    UFUNCTION(BlueprintPure, Category = "Game")
    FORCEINLINE int32 GetCurrentRound() const { return CurrentRound; }

protected:

    /** Flag to indicate if this Game Mode allows friendly fire */
    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    bool bAllowFriendlyFire;

    /** The amount of score a player gains when killing an enemy */
    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    float ScorePerKill;

    // Number of rounds needed to win to win the match
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    int32 RoundsToWin;

    /** The score needed to win the game */
    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    int32 MaxScore;

    /** Waiting time before each round */
    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    float PreRoundDuration;

    /** The max duration of a round in the game */
    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    float MaxRoundDuration;

    /** Waiting time after each round is finished */
    UPROPERTY(EditDefaultsOnly, Category = "Rules")
    float PostRoundDuration;

    /** The delay after which server will change map*/
    UPROPERTY(EditDefaultsOnly, Category = "End")
    float TravelDelay;

protected:

    /** Initialize game state default values */
    virtual void InitGameState() override;

    /** Cached GameState of this game */
    class ACSGameState* CSGameState;

private:

    /** TimerHandle for efficient management of TickGameTime */
    FTimerHandle TimerHandle_TickGameTime;

    /** TimerHandle for efficient management of LoadMainMenuMap */
    FTimerHandle TimerHandle_LoadMenuHandle;

protected:

    /** One second periodical timer for handling in game events */
    UFUNCTION()
    virtual void TickGameTime();

public:

    /** Initialize default values */
    ACSGameMode();

    /** Whether or not this Game Mode allows friendly fire */
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    bool IsFriendlyFireAllowed();

    /** Notify this GameMode about kills */
    virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

    UPROPERTY(BlueprintAssignable, Category = "GameMode")
    FOnActorKilled OnActorKilled;
};

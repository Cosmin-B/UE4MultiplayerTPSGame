// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CSAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSAIController : public AAIController
{
	GENERATED_BODY()
	
public:

    /** Initialize Default values */
    ACSAIController();

public:
    /** Begin AController interface */
    virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;

protected:

    virtual void OnPossess(class APawn* InPawn) override;
    virtual void OnUnPossess() override;
    /** End APlayerController interface*/

public:

    void CheckAmmo(const class ACSWeapon* CurrentWeapon);

protected:

    int32 NeedAmmoKeyID;

private:

    UPROPERTY(Transient)
    UBlackboardComponent* BlackboardComp;

    /* Cached BT component */
    UPROPERTY(transient)
    UBehaviorTreeComponent* BehaviorComp;

public:

    /** Returns BlackboardComp subobject **/
    FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }
    /** Returns BehaviorComp subobject **/
    FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }
};

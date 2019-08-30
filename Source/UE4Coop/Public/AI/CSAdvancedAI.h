// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CSCharacter.h"
#include "CSAdvancedAI.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSAdvancedAI : public ACSCharacter
{
	GENERATED_BODY()
	
public:

    /** Initialize Default Values */
    ACSAdvancedAI();

public:

    /** Behavior of this AI */
    UPROPERTY(EditAnywhere, Category = Behavior)
    class UBehaviorTree* AIBehavior;

};

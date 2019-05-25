// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CSAbilityTask_TestTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTestTaskTwoParamDelegate, float, FirstParam, int32, SecondParam);

/**
 * 
 */
UCLASS()
class UE4COOP_API UCSAbilityTask_TestTask : public UAbilityTask
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FTestTaskTwoParamDelegate OnCalled;

    virtual void Activate() override;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "ExecuteMyTask", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UCSAbilityTask_TestTask* CreateMyTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, float ExampleVariable);
};

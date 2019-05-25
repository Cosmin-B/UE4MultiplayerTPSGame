// Fill out your copyright notice in the Description page of Project Settings.


#include "CSAbilityTask_TestTask.h"

UCSAbilityTask_TestTask* UCSAbilityTask_TestTask::CreateMyTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, float ExampleVariable)
{
    UCSAbilityTask_TestTask* MyObj = NewAbilityTask<UCSAbilityTask_TestTask>(OwningAbility, TaskInstanceName);

    return MyObj;
}

void UCSAbilityTask_TestTask::Activate()
{
    OnCalled.Broadcast(10000.0f, 42);
}
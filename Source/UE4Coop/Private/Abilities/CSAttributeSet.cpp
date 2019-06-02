// Fill out your copyright notice in the Description page of Project Settings.


#include "CSAttributeSet.h"
#include "Components/CSHealthComponent.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UCSAttributeSet::UCSAttributeSet()
{
    const float DefaultMaxHealth = 100.0f;

    Health.SetBaseValue(DefaultMaxHealth);
    Health.SetCurrentValue(DefaultMaxHealth);

    MaxHealth.SetBaseValue(DefaultMaxHealth);
    MaxHealth.SetCurrentValue(DefaultMaxHealth);
}

void UCSAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (HealthAttribute() == Attribute)
        NewValue = FMath::Clamp(NewValue, 0.0f, MaxHealth.GetCurrentValue());
}

void UCSAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data)
{
    UAbilitySystemComponent* Source = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();

    if (HealthAttribute() == Data.EvaluatedData.Attribute)
    {
        // Get the Target Actor
        AActor* DamagedActor = nullptr;
        APlayerController* DamagedController = nullptr;

        if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
        {
            DamagedActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
            DamagedController = Data.Target.AbilityActorInfo->PlayerController.Get();
        }

        // Get the Source Actor
        AActor* AttackingActor = nullptr;
        APlayerController* AttackingController = nullptr;

        if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
        {
            AttackingActor = Source->AbilityActorInfo->AvatarActor.Get();
            AttackingController = Source->AbilityActorInfo->PlayerController.Get();
        }

        if (Health.GetCurrentValue() <= 0)
        {
            if (DamagedActor)
            {
            }
        }
    }
}

FGameplayAttribute UCSAttributeSet::HealthAttribute()
{
    static UProperty* Property = FindFieldChecked<UProperty>(UCSAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCSAttributeSet, Health));
    return FGameplayAttribute(Property);
}
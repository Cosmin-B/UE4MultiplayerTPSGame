// Fill out your copyright notice in the Description page of Project Settings.


#include "CSAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

void UCSAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (HealthAttribute() == Attribute)
        NewValue = FMath::Clamp(NewValue, 0.0f, MaxHealth);
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

        Health = FMath::Clamp(Health, 0.0f, MaxHealth);

        if (Health <= 0)
        {
            if(DamagedActor)
                UE_LOG(LogTemp, Warning, TEXT("%s Died!"), *DamagedActor->GetName());
        }
    }
}

FGameplayAttribute UCSAttributeSet::HealthAttribute()
{
    static UProperty* Property = FindFieldChecked<UProperty>(UCSAttributeSet::StaticClass(), GET_MEMBER_NAME_CHECKED(UCSAttributeSet, Health));
    return FGameplayAttribute(Property);
}
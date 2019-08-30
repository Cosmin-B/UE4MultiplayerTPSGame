// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CSAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API UCSAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:

    UCSAttributeSet();

    UPROPERTY(Category = "Character Attributes", EditAnywhere, BlueprintReadWrite)
    FGameplayAttributeData Health;

    UPROPERTY(Category = "Character Attributes", EditAnywhere, BlueprintReadWrite)
    FGameplayAttributeData MaxHealth;

public:

    virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) override;

    static FGameplayAttribute HealthAttribute();
};

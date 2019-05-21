// Fill out your copyright notice in the Description page of Project Settings.


#include "CSHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCSHealthComponent::UCSHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    MaxHealth = 100;

    SetIsReplicated(true);
}

// Called when the game starts
void UCSHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerRole() == ROLE_Authority)
    {
        AActor* MyOwner = GetOwner();
        if (MyOwner)
            MyOwner->OnTakeAnyDamage.AddDynamic(this, &UCSHealthComponent::OnDamageTaken);
    }

    Health = MaxHealth;
}

void UCSHealthComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f)
        return;

    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
}

void UCSHealthComponent::ApplyHeal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
        return;

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

void UCSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCSHealthComponent, Health);
}

void UCSHealthComponent::OnRep_Health(float OldHealth)
{
    float damage = Health - OldHealth;

    OnHealthChanged.Broadcast(this, Health, damage, nullptr, nullptr, nullptr);
}
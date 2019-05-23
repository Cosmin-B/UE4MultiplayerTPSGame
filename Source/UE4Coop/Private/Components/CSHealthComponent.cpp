// Fill out your copyright notice in the Description page of Project Settings.


#include "CSHealthComponent.h"
#include "../Public/CSGameMode.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCSHealthComponent::UCSHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    bIsDead = false;
    MaxHealth = 100;

    TeamNum = 255;

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
    if (Damage <= 0.0f || bIsDead)
        return;

    if (DamageCauser != DamageCauser && IsFriendly(DamagedActor, DamageCauser))
        return;

    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    bIsDead = Health <= 0.0f;

    if (bIsDead)
        if (ACSGameMode* GM = Cast<ACSGameMode>(GetWorld()->GetAuthGameMode()))
            GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
}

void UCSHealthComponent::ApplyHeal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
        return;

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

bool UCSHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
    if (ActorA == nullptr || ActorB == nullptr)
        return true;

    UCSHealthComponent* HealthCompA = Cast<UCSHealthComponent>(ActorA->GetComponentByClass(UCSHealthComponent::StaticClass()));
    UCSHealthComponent* HealthCompB = Cast<UCSHealthComponent>(ActorB->GetComponentByClass(UCSHealthComponent::StaticClass()));

    if (HealthCompA == nullptr || HealthCompB == nullptr)
        return true;

    return HealthCompA->TeamNum == HealthCompB->TeamNum;
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

float UCSHealthComponent::GetHealth() const
{
    return Health;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CSHealthComponent.h"
#include "CSGameMode.h"
#include "CSCharacter.h"

#include "GameFramework/DamageType.h"
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

    ACSGameMode* CSGameMode = Cast<ACSGameMode>(GetWorld()->GetAuthGameMode());
    if (!CSGameMode)
        return;

    if (DamageCauser != DamageCauser && (IsFriendly(DamagedActor, DamageCauser) && !CSGameMode->IsFriendlyFireAllowed()))
        return;

    const float OldHealth = Health;

    Health = FMath::Clamp(Health - Damage, -1.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    bIsDead = Health <= 0.0f;

    ACSCharacter* CSDamageCauser = Cast<ACSCharacter>(DamageCauser);
    if(CSDamageCauser)
        CSDamageCauser->RegisterAction(ECharacterAction::DamageDone, OldHealth - Health);

    ACSCharacter* CSOwner = Cast<ACSCharacter>(GetOwner());
    if (CSOwner)
    {
        // If owner is player send a client OnDamageTaken event broadcast
        if (CSOwner->Controller && CSOwner->Controller->PlayerState)
            ClientDamageTaken(Damage, InstigatedBy, DamageCauser);

        CSOwner->RegisterAction(ECharacterAction::DamageTaken, Damage);

        if (bIsDead)
            CSGameMode->Killed(InstigatedBy, CSOwner->Controller, CSOwner, DamageType);
    }
}

void UCSHealthComponent::ApplyHeal(float HealAmount)
{
    if (HealAmount <= 0.0f || Health <= 0.0f)
        return;

    Health = FMath::Clamp(Health + HealAmount, -1.0f, MaxHealth);

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

void UCSHealthComponent::ClientDamageTaken_Implementation(float Damage, class AController* InstigatedBy, AActor* DamageCauser)
{
    AActor* MyOwner = GetOwner();
    if (MyOwner)
        MyOwner->OnTakeAnyDamage.Broadcast(MyOwner, Damage, nullptr, InstigatedBy, DamageCauser);
}

bool UCSHealthComponent::ClientDamageTaken_Validate(float Damage, class AController* InstigatedBy, AActor* DamageCauser)
{
    return true;
}

void UCSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCSHealthComponent, bIsDead);
    DOREPLIFETIME(UCSHealthComponent, Health);
    DOREPLIFETIME(UCSHealthComponent, TeamNum);
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

float UCSHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

bool UCSHealthComponent::IsDead() const
{
    return Health <= 0;
}
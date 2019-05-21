// Fill out your copyright notice in the Description page of Project Settings.


#include "CSTrackerBot.h"
#include "../Public/CSCharacter.h"
#include "../Public/Components/CSHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

// Sets default values
ACSTrackerBot::ACSTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetCanEverAffectNavigation(false);

    RootComponent = MeshComp;

    HealthComp = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComp"));
    HealthComp->OnHealthChanged.AddDynamic(this, &ACSTrackerBot::OnDamageTaken);

    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetSphereRadius(200.0f);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    SphereComp->SetupAttachment(RootComponent);

    bExploded = false;
    bUseVelocityChange = false;

    MovementForce = 1000.0f;
    RequiredDistanceToTarget = 100.0f;

    SelfDamage = 20.0f;
    SelfDamagePeriodic = 0.5f;

    ExplosionDamage = 90.0f;
    ExplosionRadius = 200.0f;
}

// Called when the game starts or when spawned
void ACSTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
    if (Role == ENetRole::ROLE_Authority)
        NextPathPoint = GetNextPathPoint();
}

// Called every frame
void ACSTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (Role < ENetRole::ROLE_Authority || bExploded)
        return;

    float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

    if (DistanceToTarget <= RequiredDistanceToTarget) {
        NextPathPoint = GetNextPathPoint();

        DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
    }
    else
    {
        FVector ForceDirection = NextPathPoint - GetActorLocation();
        ForceDirection.Normalize();
        ForceDirection *= MovementForce;

        MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

        DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Green, false, 0.0f, 1.0f);
    }

    DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
}

FVector ACSTrackerBot::GetNextPathPoint()
{
    AActor* NearestPlayer = nullptr;

    float NearestDistance = BIG_NUMBER;

    // Find nearest player connected
    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(this, ACSCharacter::StaticClass(), FoundPlayers);

    for (AActor* PlayerActor : FoundPlayers)
    {
        if(!IsValid(PlayerActor))
            continue;

        const float TempDistance = FVector::DistSquared(GetActorLocation(), PlayerActor->GetActorLocation());
        if (NearestDistance > TempDistance)
        {
            NearestDistance = TempDistance;
            NearestPlayer = PlayerActor;
        }
    }

    if (NearestPlayer == nullptr)
        return FVector();

    UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), NearestPlayer);

    if (NavPath && NavPath->PathPoints.Num() > 1)
        return NavPath->PathPoints[1];

    return GetActorLocation();
}

void ACSTrackerBot::OnDamageTaken(UCSHealthComponent* OwningHealthComp, float Health, float Damage, const class UDamageType* DamageType,
                                  class AController* InstigatedBy, AActor* DamageCauser)
{
    if(PulsingMaterialInstance == nullptr)
        PulsingMaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

    if(PulsingMaterialInstance)
        PulsingMaterialInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);

    if (Health <= 0)
        SelfDestruct();
}

void ACSTrackerBot::SelfDestruct()
{
    if (bExploded)
        return;

    bExploded = true;

    UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

    MeshComp->SetVisibility(false, true);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    MeshComp->SetSimulatePhysics(false);

    if (Role < ENetRole::ROLE_Authority)
        return;

    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, 
                                        nullptr, IgnoredActors, this, GetInstigatorController(), true);

    SetLifeSpan(1.0f);
}

void ACSTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
    if (bStartedSelfDestruction)
        return;

    Super::NotifyActorBeginOverlap(OtherActor);

    ACSCharacter* PlayerPawn = Cast<ACSCharacter>(OtherActor);

    if (PlayerPawn == nullptr)
        return;

    bStartedSelfDestruction = true;

    if (Role == ENetRole::ROLE_Authority) {
        // Start self destruction sequence
        GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this,
                                        &ACSTrackerBot::ApplySelfDamage, SelfDamagePeriodic, true, 0.0f);
    }

    UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
}

void ACSTrackerBot::ApplySelfDamage()
{
    UGameplayStatics::ApplyDamage(this, SelfDamage, GetInstigatorController(), this, nullptr);
}
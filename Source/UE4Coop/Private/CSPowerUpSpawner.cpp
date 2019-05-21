// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPowerUpSpawner.h"
#include "CSPowerUpBase.h"
#include "CSCharacter.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

// Sets default values
ACSPowerUpSpawner::ACSPowerUpSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->SetSphereRadius(75.0f);
    RootComponent = SphereComp;

    DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
    DecalComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    DecalComp->DecalSize = FVector(64, 75, 75);
    DecalComp->SetupAttachment(RootComponent);

    SetReplicates(true);
}

// Called when the game starts or when spawned
void ACSPowerUpSpawner::BeginPlay()
{
	Super::BeginPlay();

    Respawn();
}

void ACSPowerUpSpawner::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (Role < ENetRole::ROLE_Authority)
        return;

    ACSCharacter * PlayerPawn = Cast<ACSCharacter>(OtherActor);

    if (PlayerPawn == nullptr)
        return;

    // This is running only on the server PowerUpInstance will be nullptr here on the clients
    if (PowerUpInstance)
    {
        PowerUpInstance->Activate(PlayerPawn);
        PowerUpInstance = nullptr;

        GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ACSPowerUpSpawner::Respawn, RespawnTimer, false);
    }
}

void ACSPowerUpSpawner::Respawn()
{
    if (Role < ENetRole::ROLE_Authority)
        return;

    if (PowerUpClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your blueprint"), *GetName());
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PowerUpInstance = GetWorld()->SpawnActor<ACSPowerUpBase>(PowerUpClass, GetTransform(), SpawnParams);
}
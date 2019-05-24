// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CSTrackerBot.generated.h"

class USoundCue;
class USphereComponent;
class UParticleSystem;
class UCSHealthComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class UE4COOP_API ACSTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACSTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UFUNCTION()
    void OnDamageTaken(UCSHealthComponent* OwningHealthComp, float Health, float Damage, const class UDamageType* DamageType, 
                       class AController* InstigatedBy, AActor* DamageCauser);

    void SelfDestruct();

    void ApplySelfDamage();

    void RefreshPath();

    FVector GetNextPathPoint();

protected:
    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    USphereComponent* SphereComp;

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UCSHealthComponent* HealthComp;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    UParticleSystem* ExplosionEffect;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float MovementForce;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float RequiredDistanceToTarget;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionDamage;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float ExplosionRadius;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float SelfDamage;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    float SelfDamagePeriodic;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    bool bUseVelocityChange;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* SelfDestructSound;

    UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
    USoundCue* ExplosionSound;

    FVector NextPathPoint;

    // Dynamic material to pulse on
    UMaterialInstanceDynamic* PulsingMaterialInstance;

    bool bExploded;

    bool bStartedSelfDestruction;

    FTimerHandle TimerHandle_SelfDamage;

    FTimerHandle TimerHandle_RefreshPath;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};

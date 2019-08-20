// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

USTRUCT()
struct FHitScanTrace
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FHitResult Hit;

    UPROPERTY()
    FVector_NetQuantize TraceEnd;

    UPROPERTY()
    bool bDidHit;

    UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;

    UPROPERTY()
    uint8 ReplicationCount;
};

UCLASS()
class UE4COOP_API ACSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSWeapon();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    virtual void Fire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    virtual void PlayFireEffects(FHitResult Hit, FVector TraceEnd, bool bDidHit, EPhysicalSurface SurfaceType);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName MuzzleSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* MuzzleEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* DefaultImpactEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* FleshImpactEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* TracerEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class UCameraShake> FireCamShake;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float VulnerableDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float RateOfFire;

    /* Bullet Spread In Degrees */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
    float ShootConeAngle;

    float TimeBetweenShots;

    float LastFireTime;

    FTimerHandle TimerHandle_TimeBetweenShots;

    UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

    UFUNCTION()
    void OnRep_HitScanTrace();

public:

    void StartFire();

    void StopFire();
};

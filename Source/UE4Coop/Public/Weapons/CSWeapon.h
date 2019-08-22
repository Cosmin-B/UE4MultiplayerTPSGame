// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSWeapon.generated.h"

class ACSCharacter;
class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Idle,
    Firing,
    Reloading
};

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

USTRUCT(BlueprintType)
struct FWeaponData
{
    GENERATED_USTRUCT_BODY()

    /** Infinite ammo for reloads */
    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
    bool bInfiniteAmmo;

    /** Infinite ammo in clip, no reload required */
    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
    bool bInfiniteClip;

    /** Max ammo */
    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
    int32 MaxAmmo;

    /** Clip size */
    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
    int32 AmmoPerClip;

    /** Initial clips */
    UPROPERTY(EditDefaultsOnly, Category = "Ammo")
    int32 InitialClips;

    /** Failsafe reload duration if weapon doesn't have any animation for it */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponStats")
    float NoAnimReloadDuration;

    /** Weapon range */
    UPROPERTY(EditDefaultsOnly, Category = "WeaponStats")
    float WeaponRange;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float RateOfFire;

    /** Defaults */
    FWeaponData()
    {
        bInfiniteAmmo = false;
        bInfiniteClip = false;
        MaxAmmo = 100;
        AmmoPerClip = 20;
        InitialClips = 5;
        RateOfFire = 700.0f;
        WeaponRange = 10000.0f;
    }
};

USTRUCT()
struct FWeaponAnim
{
    GENERATED_USTRUCT_BODY()

    /** Animation played on pawn (3rd person view) */
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* Pawn;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    bool bHasWeaponAnims = false;

    /** Animation played on the weapon (3rd person view)*/
    UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (EditCondition = "bHasWeaponAnims"))
    UAnimSequence* Weapon;
};

UCLASS()
class UE4COOP_API ACSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSWeapon();

protected:

    /** Begin AActor Interface */
    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    /** End AActor Interface */

public:

    //////////////////////////////////////////////////////////////////////////
    // Input

    /** [local + server] Start fire called by pawn */
    void StartFire();

    /** [local + server] Stop fire called by pawn */
    void StopFire();

    /** [all] Start reload called by the pawn */
    void StartReload(bool bFromReplication = false);

    /** [local + server] Interrupt weapon reload */
    virtual void StopReload();

protected:

    //////////////////////////////////////////////////////////////////////////
    // Input - server side

    UFUNCTION(Reliable, server, WithValidation)
    void ServerStartFire();

    UFUNCTION(Reliable, server, WithValidation)
    void ServerStopFire();

    UFUNCTION(Reliable, server, WithValidation)
    void ServerStartReload();

    UFUNCTION(Reliable, server, WithValidation)
    void ServerStopReload();

protected:

    //////////////////////////////////////////////////////////////////////////
    // Reload && Ammo System

    /** [server] Performs actual reload */
    virtual void ReloadWeapon();

    /** Consume a bullet */
    void UseAmmo();

protected:

    //////////////////////////////////////////////////////////////////////////
    // Animation

    /** Play weapon animations on the character*/
    float PlayAnimation(const FWeaponAnim& Animation, float PlayRate = 1.0f);

    /** Cancel playing weapon animations on the character */
    void StopAnimation(const FWeaponAnim& Animation);

    /** Play weapon animations on the weapon */
    float PlayWeaponAnimation(const FWeaponAnim& Animation, float PlayRate = 1.0f);

    /** Cancel weapon animations on the weapon */
    void StopWeaponAnimation(const FWeaponAnim& Animation);

public:

    //////////////////////////////////////////////////////////////////////////
    // Control

    /** Check if weapon can fire */
    bool CanFire() const;

    /** Check if weapon can be reloaded */
    bool CanReload() const;

    //////////////////////////////////////////////////////////////////////////
    // Reading data

    /** Check if weapon has infinite ammo*/
    bool HasInfiniteAmmo() const;

    /** Check if weapon has infinite clip */
    bool HasInfiniteClip() const;

    /** Check if weapon is reloading*/
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool IsReloading() const;

    /** Current Weapon Range */
    UFUNCTION(BlueprintCallable, Category = "WeaponStats")
    float GetWeaponRange() const;

    /** Get total ammo */
    UFUNCTION(BlueprintPure, Category = "WeaponStats")
    float GetCurrentAmmo() const;

    /** Get current ammo in clip */
    UFUNCTION(BlueprintPure, Category = "WeaponStats")
    float GetCurrentAmmoInClip() const;

    /** Get Ammo in magazine */
    UFUNCTION(BlueprintPure, Category = "WeaponStats")
    float GetCurrentAmmoInMagazine() const;

    /** Get current weapon state */
    EWeaponState GetCurrentState() const;

public:

    //////////////////////////////////////////////////////////////////////////
    // Weapon Usage

    /** [server] Owner pawn is equipping weapon */
    virtual void OnEquip(ACSCharacter* Character);

protected:

    /** [local + server] Firing started */
    virtual void OnFireStarted();

    /** [local + server] Firing finished */
    virtual void OnFireFinished();

    /** [server] Fire & update ammo */
    UFUNCTION(Reliable, server, WithValidation)
    void ServerHandleFiring();

    /** [local + server] Handle weapon fire */
    void HandleFiring();

    /** [server + local] Fire the weapon, do damage and play fire FX */
    virtual void Fire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();

    /** Update weapon state */
    void SetWeaponState(EWeaponState NewState);

    /** Determine current weapon state */
    void DetermineWeaponState();

    /** [local] Player fire FX */
    virtual void PlayFireEffects(FHitResult Hit, FVector TraceEnd, bool bDidHit, EPhysicalSurface SurfaceType);

protected:

    //////////////////////////////////////////////////////////////////////////
    // Replication

    /** Play Fire FX on remote clients */
    UFUNCTION()
    void OnRep_HitScanTrace();

    /** Start reload on remote clients too */
    UFUNCTION()
    void OnRep_Reload();

protected:

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

    //////////////////////////////////////////////////////////////////////////
    // Fire System

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class UCameraShake> FireCamShake;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float BaseDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float VulnerableDamage;

    /* Bullet Spread In Degrees */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
    float ShootConeAngle;

    float TimeBetweenShots;

    float LastFireTime;

    /** Handle for efficient management of HandleFiring timer */
    FTimerHandle TimerHandle_HandleFiring;

    UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

    /** Is weapon fire active? */
    bool bWantsToFire;

    /** Current weapon state */
    EWeaponState CurrentState;

    //////////////////////////////////////////////////////////////////////////
    // Reload && Ammo System

    /** Reload animations */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    FWeaponAnim ReloadAnim;

    /** Is reload animation playing? */
    UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
    bool bPendingReload;

    /** Is weapon currently reloading?*/
    UPROPERTY(Transient, Replicated)
    bool bReloading;

    /** Current ammo inside magazine*/
    UPROPERTY(Transient, Replicated)
    int32 CurrentAmmoInMagazine;

    /** Current total ammo */
    UPROPERTY(Transient, Replicated)
    int32 CurrentAmmo;

    /** Current ammo - inside clip */
    UPROPERTY(Transient, Replicated)
    int32 CurrentAmmoInClip;

    /** Handle for efficient management of StopReload timer */
    FTimerHandle TimerHandle_StopReload;

    /** Handle for efficient management of ReloadWeapon timer */
    FTimerHandle TimerHandle_ReloadWeapon;

protected:

    /** Weapon data */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FWeaponData WeaponConfig;

    /** Pawn owning this weapon */
    UPROPERTY(Transient, Replicated)
    ACSCharacter* MyPawn;
};

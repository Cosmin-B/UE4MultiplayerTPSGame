// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWeapon.h"
#include "CSCharacter.h"
#include "CSTypes.h"
#include "CSPlayerState.h"
#include "CSHealthComponent.h"

#include "Animation/AnimSequence.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet\GameplayStatics.h"
#include "Components\SkeletalMeshComponent.h"
#include "Particles\ParticleSystemComponent.h"
#include "Particles\ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
    TEXT("COOP.DebugWeapons"), 
    DebugWeaponDrawing, 
    TEXT("Draw Debug Lines for Weapons"), 
    ECVF_Cheat);

// Sets default values
ACSWeapon::ACSWeapon()
{
    MeshComp            = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent       = MeshComp;

    MuzzleSocketName    = "MuzzleSocket";

    BaseDamage          = 20.0f;
    ShootConeAngle      = 2.0f;
    VulnerableDamage    = BaseDamage * 2.5f;

    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;

    MyPawn = nullptr;

    bWantsToFire = false;

    SetReplicates(true);
}

void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
    // Calculate the time between shots base on the formula 60 / WeaponRateOfFire
    TimeBetweenShots = 60 / WeaponConfig.RateOfFire;
}

void ACSWeapon::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (WeaponConfig.InitialClips)
    {
        CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
        CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
        CurrentAmmoInMagazine = CurrentAmmo - CurrentAmmoInClip;
    }
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACSWeapon::StartFire()
{
    if (!HasAuthority())
        ServerStartFire();

    if (!bWantsToFire)
    {
        bWantsToFire = true;

        DetermineWeaponState();
    }
}

void ACSWeapon::StopFire()
{
    if ((!HasAuthority()) && MyPawn && MyPawn->IsLocallyControlled())
        ServerStopFire();

    if (bWantsToFire)
    {
        bWantsToFire = false;

        DetermineWeaponState();
    }
}

void ACSWeapon::StartReload(bool bFromReplication /*= false*/)
{
    if (!bFromReplication && !HasAuthority())
        ServerStartReload();

    if (bFromReplication || CanReload())
    {
        bPendingReload = true;

        DetermineWeaponState();

        float AnimDuration = PlayAnimation(ReloadAnim, 1.0f);

        if (AnimDuration <= 0.0f)
            AnimDuration = WeaponConfig.NoAnimReloadDuration;

        GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ACSWeapon::StopReload, AnimDuration, false);

        if (HasAuthority())
        {
            if (MyPawn)
                MyPawn->SetAiming(false);

            bReloading = true;
            GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ACSWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
        }
    }
}

void ACSWeapon::StopReload()
{
    if (CurrentState == EWeaponState::Reloading)
    {
        bReloading = false;
        bPendingReload = false;

        DetermineWeaponState();

        StopAnimation(ReloadAnim);

        if(MyPawn)
            MyPawn->OnStopReload();
    }
}

//////////////////////////////////////////////////////////////////////////
// Input - server side

void ACSWeapon::ServerStartFire_Implementation()
{
    StartFire();
}

bool ACSWeapon::ServerStartFire_Validate()
{
    return true;
}

void ACSWeapon::ServerStartReload_Implementation()
{
    StartReload();
}

bool ACSWeapon::ServerStartReload_Validate()
{
    return true;
}

void ACSWeapon::ServerStopReload_Implementation()
{
    StopReload();
}

bool ACSWeapon::ServerStopReload_Validate()
{
    return true;
}

void ACSWeapon::ServerStopFire_Implementation()
{
    StopFire();
}

bool ACSWeapon::ServerStopFire_Validate()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Reload && Ammo System

void ACSWeapon::ReloadWeapon()
{
    int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

    if (HasInfiniteClip())
        ClipDelta = WeaponConfig.AmmoPerClip - CurrentAmmoInClip;

    if (ClipDelta > 0)
        CurrentAmmoInClip += ClipDelta;

    if (HasInfiniteClip())
        CurrentAmmo = FMath::Max(CurrentAmmoInClip, CurrentAmmo);

    if (!HasInfiniteAmmo())
        CurrentAmmoInMagazine = FMath::Clamp(CurrentAmmoInMagazine - ClipDelta, 0, CurrentAmmoInMagazine);
}

void ACSWeapon::UseAmmo()
{
    if (!HasInfiniteClip())
        CurrentAmmoInClip--;

    if (!HasInfiniteAmmo())
        CurrentAmmo--;
}

//////////////////////////////////////////////////////////////////////////
// Animation

float ACSWeapon::PlayAnimation(const FWeaponAnim& Animation, float PlayRate /*= 1.0f*/)
{
    float Duration = 0.0f;
    if (MyPawn)
    {
        UAnimMontage* UseAnim = Animation.Pawn;

        if (UseAnim)
            Duration = MyPawn->PlayAnimMontage(UseAnim, PlayRate) * PlayRate;

        if (Animation.bHasWeaponAnims)
        {
            float TempDuration = PlayWeaponAnimation(Animation, PlayRate);

            if (TempDuration > Duration)
                Duration = TempDuration;
        }
    }

    return Duration;
}

void ACSWeapon::StopAnimation(const FWeaponAnim& Animation)
{
    if (MyPawn)
    {
        UAnimMontage* UseAnim = Animation.Pawn;

        if (UseAnim)
            MyPawn->StopAnimMontage(UseAnim);

        if (Animation.bHasWeaponAnims)
            StopWeaponAnimation(Animation);
    }
}

float ACSWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation, float PlayRate /*= 1.0f*/)
{
    float Duration = 0.5f;

    USkeletalMeshComponent* UseMesh = MeshComp;
    UAnimSequence* UseAnim = Animation.Weapon;

    if (UseMesh && UseAnim)
    {
        UseMesh->PlayAnimation(UseAnim, false);
        UseAnim->RateScale = PlayRate;
        Duration = PlayRate * UseAnim->GetPlayLength();
    }

    return Duration;
}

void ACSWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
    USkeletalMeshComponent* UseMesh = MeshComp;
    UAnimSequence* UseAnim =Animation.Weapon;

    if (UseMesh && UseAnim)
        UseMesh->PlayAnimation(nullptr, false);
}

//////////////////////////////////////////////////////////////////////////
// Control

bool ACSWeapon::CanFire() const
{
    bool bCanFire = MyPawn && MyPawn->CanFire();
    bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
    return (bCanFire && bStateOKToFire && !bPendingReload);
}

bool ACSWeapon::CanReload() const
{
    bool bCanReload = (!MyPawn || MyPawn->CanReload());
    bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteAmmo());
    bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
    return (bCanReload && bGotAmmo && bStateOKToReload);
}

//////////////////////////////////////////////////////////////////////////
// Weapon Usage

void ACSWeapon::OnEquip(ACSCharacter* Character)
{
    SetOwner(Character);

    AttachToComponent(
        Character->GetMesh(), 
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
        Character->GetWeaponAttachPoint());

    MyPawn = Character;
}

void ACSWeapon::OnFireStarted()
{
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

    GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ACSWeapon::HandleFiring, TimeBetweenShots, true, FirstDelay);
}

void ACSWeapon::OnFireFinished()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
}

void ACSWeapon::ServerHandleFiring_Implementation()
{
    const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

    HandleFiring();

    if (bShouldUpdateAmmo)
    {
        // Update ammo
        UseAmmo();
    }
}

bool ACSWeapon::ServerHandleFiring_Validate()
{
    return true;
}

bool ACSWeapon::ServerFire_Validate()
{
    return true;
}

void ACSWeapon::ServerFire_Implementation()
{
    Fire();
}

void ACSWeapon::HandleFiring()
{
    if ((CurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
    {
        if (MyPawn && MyPawn->IsLocallyControlled())
        {
            Fire();

            UseAmmo();
        }
    }
    else if (CanReload())
        StartReload();

    if (MyPawn && MyPawn->IsLocallyControlled())
    {
        if (MyPawn->Role < ENetRole::ROLE_Authority)
            ServerHandleFiring();

        // Reload after firing last round
        if (CurrentAmmoInClip <= 0 && CanReload())
            StartReload();
    }

    LastFireTime = GetWorld()->GetTimeSeconds();
}

void ACSWeapon::Fire()
{
    if (!MyPawn || !CanFire())
        return;

    if (Role < ROLE_Authority)
        ServerFire();

    FVector EyeLocation;
    FRotator EyeRotation;
    MyPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

    FVector ShotDirection = EyeRotation.Vector();

    float HalfConeAngle = FMath::DegreesToRadians(ShootConeAngle * 0.5f);
    ShotDirection = FMath::VRandCone(ShotDirection, HalfConeAngle, HalfConeAngle);
    FVector TraceEnd = EyeLocation + (ShotDirection * WeaponConfig.WeaponRange);

    FCollisionQueryParams QueryParams;

    QueryParams.AddIgnoredActor(MyPawn);
    QueryParams.AddIgnoredActor(this);

    QueryParams.bTraceComplex           = true;
    QueryParams.bReturnPhysicalMaterial = true;

    if (DebugWeaponDrawing)
        DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

    FHitResult Hit;

    // TODO: Do a better hit confirmation
    bool bDidHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);

    EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType_Default;

    if (bDidHit && MyPawn->HasAuthority())
    {
        AActor* HitActor = Hit.GetActor();

        SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

        float FinalDamage = SurfaceType == SURFACE_FLESHVULNERABLE ? VulnerableDamage : BaseDamage;

        UGameplayStatics::ApplyPointDamage(HitActor, FinalDamage, ShotDirection, Hit, MyPawn->Controller, MyPawn, DamageType);

        if (MyPawn && HitActor && HitActor != MyPawn && HitActor->GetComponentByClass(UCSHealthComponent::StaticClass()))
            MyPawn->RegisterAction(ECharacterAction::ShotHit);
    }

    PlayFireEffects(Hit, TraceEnd, bDidHit, SurfaceType);

    if (Role == ROLE_Authority)
    {
        if (MyPawn)
            MyPawn->RegisterAction(ECharacterAction::ShotFire);

        HitScanTrace.Hit = Hit;
        HitScanTrace.TraceEnd = TraceEnd;
        HitScanTrace.bDidHit = bDidHit;
        HitScanTrace.ReplicationCount = HitScanTrace.ReplicationCount + 1;
    }
}

void ACSWeapon::DetermineWeaponState()
{
    EWeaponState NewState = EWeaponState::Idle;

    if (bPendingReload)
    {
        if (!CanReload())
            NewState = CurrentState;
        else
            NewState = EWeaponState::Reloading;
    }
    else if (!bPendingReload && bWantsToFire && CanFire())
        NewState = EWeaponState::Firing;

    SetWeaponState(NewState);
}

void ACSWeapon::SetWeaponState(EWeaponState NewState)
{
    const EWeaponState PrevState = CurrentState;

    if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
        OnFireFinished();

    CurrentState = NewState;

    if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
        OnFireStarted();
}

void ACSWeapon::PlayFireEffects(FHitResult Hit, FVector TraceEnd, bool bDidHit, EPhysicalSurface SurfaceType)
{
    if (MuzzleEffect)
        UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);

    if (TracerEffect)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

        if (TracerComp)
            TracerComp->SetVectorParameter("BeamEnd", bDidHit ? Hit.ImpactPoint : TraceEnd);
    }

    if (MyPawn)
    {
        APlayerController* PlayerController = Cast<APlayerController>(MyPawn->GetController());

        if (PlayerController)
            PlayerController->ClientPlayCameraShake(FireCamShake);
    }

    if (!bDidHit)
        return;

    UParticleSystem* SelectedEffect = nullptr;

    switch (SurfaceType)
    {
        case SURFACE_FLESHDEFAULT:
        case SURFACE_FLESHVULNERABLE:
            SelectedEffect = FleshImpactEffect;
            break;
        default:
            SelectedEffect = DefaultImpactEffect;
            break;
    }

    if (SelectedEffect)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
}

//////////////////////////////////////////////////////////////////////////
// Reading data

bool ACSWeapon::HasInfiniteAmmo() const
{
    return WeaponConfig.bInfiniteAmmo;
}

bool ACSWeapon::HasInfiniteClip() const
{
    return WeaponConfig.bInfiniteClip;
}

bool ACSWeapon::IsReloading() const
{
    return bReloading;
}

float ACSWeapon::GetWeaponRange() const
{
    return WeaponConfig.WeaponRange;
}

EWeaponState ACSWeapon::GetCurrentState() const
{
    return CurrentState;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ACSWeapon::OnRep_HitScanTrace()
{
    PlayFireEffects(HitScanTrace.Hit, HitScanTrace.TraceEnd, HitScanTrace.bDidHit, HitScanTrace.SurfaceType);
}

void ACSWeapon::OnRep_Reload()
{
    if (bPendingReload)
        StartReload(true);
    else
        StopReload();
}

void ACSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ACSWeapon, MyPawn);

    // Replicate to local owner only
    DOREPLIFETIME_CONDITION(ACSWeapon, bReloading, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACSWeapon, CurrentAmmo, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACSWeapon, CurrentAmmoInClip, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACSWeapon, CurrentAmmoInMagazine, COND_OwnerOnly);

    // Replicate to everyone except the local owner
    DOREPLIFETIME_CONDITION(ACSWeapon, HitScanTrace, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(ACSWeapon, bPendingReload, COND_SkipOwner);
}
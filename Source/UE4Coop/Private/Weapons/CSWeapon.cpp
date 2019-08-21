// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWeapon.h"
#include "CSCharacter.h"
#include "CSTypes.h"
#include "CSPlayerState.h"
#include "CSHealthComponent.h"

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

    RateOfFire          = 900;
    BaseDamage          = 20.0f;
    ShootConeAngle      = 2.0f;
    VulnerableDamage    = BaseDamage * 2.5f;

    NetUpdateFrequency = 66.0f;
    MinNetUpdateFrequency = 33.0f;

    MyPawn = nullptr;

    SetReplicates(true);
}

void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
    TimeBetweenShots = 60 / RateOfFire;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACSWeapon::StartFire()
{
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ACSWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
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

void ACSWeapon::Fire()
{
    if (Role < ROLE_Authority)
        ServerFire();
    
    if (!MyPawn)
        return;

    FVector EyeLocation;
    FRotator EyeRotation;
    MyPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

    FVector ShotDirection   = EyeRotation.Vector();

    float halfConeAngle = FMath::DegreesToRadians(ShootConeAngle * 0.5f);

    ShotDirection = FMath::VRandCone(ShotDirection, halfConeAngle, halfConeAngle);

    FVector TraceEnd        = EyeLocation + (ShotDirection * 10000);

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

    LastFireTime = GetWorld()->TimeSeconds;
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

bool ACSWeapon::ServerFire_Validate()
{
    return true;
}

void ACSWeapon::ServerFire_Implementation()
{
    Fire();
}

void ACSWeapon::OnRep_HitScanTrace()
{
    PlayFireEffects(HitScanTrace.Hit, HitScanTrace.TraceEnd, HitScanTrace.bDidHit, HitScanTrace.SurfaceType);
}

void ACSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate to everyone
    DOREPLIFETIME(ACSWeapon, MyPawn);

    // Replicate to everyone except the local owner
    DOREPLIFETIME_CONDITION(ACSWeapon, HitScanTrace, COND_SkipOwner);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/CSWeapon.h"
#include "../UE4Coop.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet\GameplayStatics.h"
#include "Components\SkeletalMeshComponent.h"
#include "Particles\ParticleSystemComponent.h"
#include "Particles\ParticleSystem.h"
#include "DrawDebugHelpers.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
    TEXT("COOP.DebugWeapons"), 
    DebugWeaponDrawing, 
    TEXT("Draw Debug Lines for Weapons"), 
    ECVF_Cheat);

// Sets default values
ACSWeapon::ACSWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    MuzzleSocketName = "MuzzleSocket";
}

// Called when the game starts or when spawned
void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACSWeapon::Fire()
{
    AActor* MyOwner = GetOwner();
    
    if (MyOwner == nullptr)
        return;

    FVector EyeLocation;
    FRotator EyeRotation;
    MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

    FVector ShotDirection   = EyeRotation.Vector();
    FVector TraceEnd        = EyeLocation + (ShotDirection * 10000);

    FCollisionQueryParams QueryParams;

    QueryParams.AddIgnoredActor(MyOwner);
    QueryParams.AddIgnoredActor(this);

    QueryParams.bTraceComplex           = true;
    QueryParams.bReturnPhysicalMaterial = true;

    if (DebugWeaponDrawing)
        DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

    FHitResult Hit;

    bool didHit = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams);

    if (didHit)
    {
        AActor* HitActor = Hit.GetActor();

        UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

        EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

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

    PlayFireEffects(Hit, TraceEnd, didHit);
}

void ACSWeapon::PlayFireEffects(FHitResult Hit, FVector TraceEnd, bool bDidHit)
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

    APawn* MyOwner = Cast<APawn>(GetOwner());

    if (MyOwner)
    {
        APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());

        if (PlayerController)
            PlayerController->ClientPlayCameraShake(FireCamShake);
    }
}

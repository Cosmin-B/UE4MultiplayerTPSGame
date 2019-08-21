// Fill out your copyright notice in the Description page of Project Settings.

#include "CSCharacter.h"
#include "CSWeapon.h"
#include "CSTypes.h"
#include "Components/CSHealthComponent.h"
#include "Abilities/CSAttributeSet.h"
#include "CSPlayerState.h"

#include "Animation/AnimMontage.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACSCharacter::ACSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->SetupAttachment(RootComponent);

    GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

    GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp);

    HealthComp = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComp"));

    // Our ability system component
    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    AttributeSet = CreateDefaultSubobject<UCSAttributeSet>(TEXT("AttributeSet"));

    ZoomedFOV = 65.5f;
    ZoomInterpSpeed = 20.0f;

    WeaponAttachSocketName = "WeaponSocket";
}

//////////////////////////////////////////////////////////////////////////
// ACharacter Interface

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();

    DefaultFOV = CameraComp->FieldOfView;

    HealthComp->OnHealthChanged.AddDynamic(this, &ACSCharacter::OnHealthChanged);

    if (HasAuthority())
    {
        bDied = false;

        // Spawn a default weapon
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        CurrentWeapon = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

        if (CurrentWeapon)
            CurrentWeapon->OnEquip(this);
    }
}

void ACSCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystem)
        AbilitySystem->RefreshAbilityActorInfo();
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
    float NewFOV    = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

    CameraComp->SetFieldOfView(NewFOV);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveRight", this, &ACSCharacter::MoveRight);
    PlayerInputComponent->BindAxis("MoveForward", this, &ACSCharacter::MoveForward);

    PlayerInputComponent->BindAxis("LookUp", this, &ACSCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("Turn", this, &ACSCharacter::AddControllerYawInput);

    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACSCharacter::BeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACSCharacter::EndCrouch);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACSCharacter::MoveJump);

    PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ACSCharacter::BeginZoom);
    PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ACSCharacter::EndZoom);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSCharacter::StartFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACSCharacter::StopFire);

    AbilitySystem->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "EAbilityInput"));
}

void ACSCharacter::AcquireAbility(TSubclassOf<UGameplayAbility> AbiltyToAcquire)
{
    if (AbilitySystem)
    {
        if (HasAuthority() && AbiltyToAcquire)
            AbilitySystem->GiveAbility(FGameplayAbilitySpec(AbiltyToAcquire, 1, 0));

        AbilitySystem->InitAbilityActorInfo(this, this);
    }
}

void ACSCharacter::MoveForward(float Value)
{
    AddMovementInput(GetActorForwardVector() * Value);
}

void ACSCharacter::MoveRight(float Value)
{
    AddMovementInput(GetActorRightVector() * Value);
}

void ACSCharacter::MoveJump()
{
    Jump();
}

void ACSCharacter::BeginCrouch()
{
    Crouch();
}

void ACSCharacter::EndCrouch()
{
    UnCrouch();
}

void ACSCharacter::BeginZoom()
{
    bWantsToZoom = true;
}

void ACSCharacter::EndZoom()
{
    bWantsToZoom = false;
}

void ACSCharacter::StartFire()
{
    if (CurrentWeapon)
        CurrentWeapon->StartFire();
}

void ACSCharacter::StopFire()
{
    if (CurrentWeapon)
        CurrentWeapon->StopFire();
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

bool ACSCharacter::CanFire() const
{
    return !bDied;
}

bool ACSCharacter::CanReload() const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Animations

float ACSCharacter::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate /*= 1.f*/, FName StartSectionName /*= NAME_None*/)
{
    USkeletalMeshComponent* UseMesh = GetMesh();

    if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
        return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);

    return 0.0f;
}

void ACSCharacter::StopAnimMontage(UAnimMontage* AnimMontage)
{
    USkeletalMeshComponent* UseMesh = GetMesh();

    if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
        UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
    {
        UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
    }
}

void ACSCharacter::StopAllAnimMontages()
{
    USkeletalMeshComponent* UseMesh = GetMesh();

    if (UseMesh && UseMesh->AnimScriptInstance)
        UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
}

//////////////////////////////////////////////////////////////////////////
// Statistics

void ACSCharacter::RegisterAction(ECharacterAction Action, float Amount /*= 0*/)
{
    if (!HasAuthority())
        return;

    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (PlayerController)
    {
        ACSPlayerState* CSPlayerState = Cast<ACSPlayerState>(PlayerController->PlayerState);
        if (CSPlayerState)
        {
            switch (Action)
            {
                case ECharacterAction::ShotFire:
                    CSPlayerState->RegisterShotFired();
                    break;
                case ECharacterAction::ShotHit:
                    CSPlayerState->RegisterShotHit();
                    break;
                case ECharacterAction::DamageDone:
                    CSPlayerState->RegisterDamageDone((int32)Amount);
                    break;
                case ECharacterAction::DamageTaken:
                    CSPlayerState->RegisterDamageTaken((int32)Amount);
                    break;
                default:
                    UE_LOG(LogTemp, Warning, TEXT("Wanted to register unhandled character action"));
                    break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Damage and health system

void ACSCharacter::OnHealthChanged(UCSHealthComponent* HealthComponent, float Health, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bDied)
    {
        bDied = true;

        GetMovementComponent()->StopMovementImmediately();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        DetachFromControllerPendingDestroy();

        SetLifeSpan(10.0f);
    }
}

//////////////////////////////////////////////////////////////////////////
// Reading Data

FName ACSCharacter::GetWeaponAttachPoint() const
{
    return WeaponAttachSocketName;
}

FVector ACSCharacter::GetPawnViewLocation() const
{
    if (CameraComp)
        return CameraComp->GetComponentLocation();

    return Super::GetPawnViewLocation();
}

UAbilitySystemComponent* ACSCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystem;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ACSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSCharacter, CurrentWeapon);
    DOREPLIFETIME(ACSCharacter, bDied);
}
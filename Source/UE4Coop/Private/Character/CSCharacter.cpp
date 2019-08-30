// Fill out your copyright notice in the Description page of Project Settings.

#include "CSCharacter.h"
#include "CSWeapon.h"
#include "CSTypes.h"
#include "Components/CSHealthComponent.h"
#include "Abilities/CSAttributeSet.h"
#include "CSPlayerState.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimMontage.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

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

    bWasAiming = false;
    bAiming = false;

    WeaponAttachSocketName = "WeaponSocket";
}

//////////////////////////////////////////////////////////////////////////
// ACharacter Interface

void ACSCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // Create material instance for setting team colors
    for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
        MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));

    UpdateTeamColorsAllMIDs();
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();

    DefaultFOV = CameraComp->FieldOfView;

    HealthComp->OnHealthChanged.AddDynamic(this, &ACSCharacter::OnHealthChanged);

    if (HasAuthority())
    {
        bDied = false;

        // TODO: Make this a member variable and expose it to blueprint
        const float DelayToSpawnDefaultWeapon = 0.5f;

        FTimerHandle TimerHandle_SpawnDefaultWeapon;
        GetWorldTimerManager().SetTimer(TimerHandle_SpawnDefaultWeapon, this, &ACSCharacter::SpawnDefaultWeapon, DelayToSpawnDefaultWeapon, false);
    }

    // [all] after healthcomp teamnum is assigned, set team colors of this pawn
    UpdateTeamColorsAllMIDs();
}

void ACSCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystem)
        AbilitySystem->RefreshAbilityActorInfo();

    // [server] after healthcomp teamnum is assigned, set team colors of this pawn
    UpdateTeamColorsAllMIDs();
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    float TargetFOV = IsAiming() ? ZoomedFOV : DefaultFOV;
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

    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACSCharacter::OnStartReload);

    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACSCharacter::BeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACSCharacter::EndCrouch);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACSCharacter::MoveJump);

    PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ACSCharacter::OnStartAiming);
    PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ACSCharacter::OnStopAiming);

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

void ACSCharacter::OnStartReload()
{
    if (CurrentWeapon)
    {
        // Disable aiming while reloading
        if (CurrentWeapon->CanReload())
            SetAiming(false);

        CurrentWeapon->StartReload();
    }
}

void ACSCharacter::OnStopReload()
{
    SetAiming(bWasAiming);
}

void ACSCharacter::BeginCrouch()
{
    Crouch();
}

void ACSCharacter::EndCrouch()
{
    UnCrouch();
}

void ACSCharacter::OnStartAiming()
{
    SetAiming(true);
}

void ACSCharacter::OnStopAiming()
{
    SetAiming(false);
    bWasAiming = false;
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

void ACSCharacter::SpawnDefaultWeapon()
{
    // Spawn a default weapon
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ACSWeapon* Weapon = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    EquipWeapon(Weapon);
}

void ACSCharacter::EquipWeapon(ACSWeapon* NewWeapon)
{
    if (!HasAuthority())
        return;

    // TODO: Unequip last weapon

    CurrentWeapon = NewWeapon;

    if (CurrentWeapon)
        CurrentWeapon->OnEquip(this);

    OnWeaponEquip.Broadcast(this, CurrentWeapon);
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
// Materials

void ACSCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
    if (UseMID)
    {
        if (HealthComp)
        {
            float MaterialParam = (float)HealthComp->TeamNum;
            UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
        }
    }
}

void ACSCharacter::UpdateTeamColorsAllMIDs()
{
    for (int32 i = 0; i < MeshMIDs.Num(); ++i)
        UpdateTeamColors(MeshMIDs[i]);
}

//////////////////////////////////////////////////////////////////////////
// Character Aiming

void ACSCharacter::SetAiming(bool bNewAiming)
{
    if (bNewAiming)
    {
        // Don't aim down sights if the character is not holding any weapon
        if (!CurrentWeapon)
            return;

        // Don't aim if the character is reloading
        if (CurrentWeapon->IsReloading())
            return;
    }

    bWasAiming = bAiming;

    bAiming = bNewAiming;

    if (!HasAuthority())
        ServerSetAiming(bNewAiming);

    OnAimStateChange.Broadcast(this, bNewAiming);
}

void ACSCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    SetAiming(bNewAiming);
}

bool ACSCharacter::ServerSetAiming_Validate(bool bNewAiming)
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Damage and health system

void ACSCharacter::OnHealthChanged(UCSHealthComponent* HealthComponent, float Health, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Health <= 0.0f && !bDied)
    {
        bDied = true;

        DetachFromControllerPendingDestroy();

        GetMovementComponent()->StopMovementImmediately();

        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

        SetLifeSpan(5.0f);
    }
}

//////////////////////////////////////////////////////////////////////////
// Reading Data

bool ACSCharacter::IsAiming() const
{
    return bAiming;
}

bool ACSCharacter::IsAlive() const
{
    return !HealthComp->IsDead();
}

ACSWeapon* ACSCharacter::GetCurrentWeapon() const
{
    return CurrentWeapon;
}

UCSHealthComponent* ACSCharacter::GetHealthComponent() const
{
    return HealthComp;
}

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

void ACSCharacter::OnRep_CurrentWeapon()
{
    if (Controller && Controller->IsLocalController())
        OnWeaponEquip.Broadcast(this, CurrentWeapon);
}

void ACSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSCharacter, CurrentWeapon);
    DOREPLIFETIME(ACSCharacter, bDied);
    DOREPLIFETIME(ACSCharacter, bAiming);
}
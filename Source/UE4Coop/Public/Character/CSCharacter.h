// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "CSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UCSHealthComponent;
class ACSWeapon;
class UGameplayAbility;
class UCSAttributeSet;
class UAbilitySystemComponent;

/** Event for aim state being changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterAimStateChangedSignature, ACSCharacter*, Character, bool, NewState);

/** Event for aim state being changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharactereEquipWeaponSignature, ACSCharacter*, Character, ACSWeapon*, NewWeapon);

UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
    UseAbility1 UMETA(DisplayName = "Use Spell 1"),
    UseAbility2 UMETA(DisplayName = "Use Spell 2"),
    UseAbility3 UMETA(DisplayName = "Use Spell 3"),
    UseAbility4 UMETA(DisplayName = "Use Spell 4"),
};

UENUM(BlueprintType)
enum class ECharacterAction : uint8
{
    ShotFire        UMETA(DisplayName = "Shot Fire"),
    ShotHit         UMETA(DisplayName = "Shot Hit"),
    DamageDone      UMETA(DisplayName = "Damage Done"),
    DamageTaken     UMETA(DisplayName = "Damage Taken"),

    MaxAction UMETA(Hidden),
};

UCLASS()
class UE4COOP_API ACSCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

    UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:

	// Sets default values for this character's properties
	ACSCharacter();

protected:

    /** Begin ACharacter Interface */
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual FVector GetPawnViewLocation() const override;
    /** End ACharacter Interface */

protected:

    //////////////////////////////////////////////////////////////////////////
    // Input

    /**
    * Move forward/back
    *
    * @param Value Movement input to apply
    */
    void MoveForward(float Value);

    /**
    * Strafe right/left
    *
    * @param Value Movement input to apply
    */
    void MoveRight(float Value);

    /** Player pressed jump action */
    void MoveJump();

    /** Player pressed crouch action */
    void BeginCrouch();

    /** Player released crouch action */
    void EndCrouch();

    /** Player pressed reload action */
    void OnStartReload();

    /** Player pressed Aim Down Sights action */
    void OnStartAiming();

    /** Player released Aim Down Sights action */
    void OnStopAiming();

    /** Player pressed fire action */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void StartFire();

    /** Player released fire action */
    UFUNCTION(BlueprintCallable, Category = "Player")
    void StopFire();

public:

    //////////////////////////////////////////////////////////////////////////
    // Weapon usage

    /** Check if pawn can fire weapon */
    bool CanFire() const;

    /** Check if pawn can reload weapon */
    bool CanReload() const;

    /** Spawn default weapon */
    virtual void SpawnDefaultWeapon();

    /** Equip New Weapon and broadcast the event */
    virtual void EquipWeapon(ACSWeapon* NewWeapon);

    //////////////////////////////////////////////////////////////////////////
    // Animations

    /** Play anim montage */
    virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

    /** Stop playing montage */
    virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

    /** Stop playing all montages */
    void StopAllAnimMontages();

public:

    /** Performs character checks when the reloading is complete (Not input related)*/
    void OnStopReload();

    //////////////////////////////////////////////////////////////////////////
    // Camera

    /** [server + local] change targeting state */
    void SetAiming(bool bNewAiming);

protected:

    /** Update targeting state */
    UFUNCTION(Reliable, server, WithValidation)
    void ServerSetAiming(bool bNewAiming);

    //////////////////////////////////////////////////////////////////////////
    // Damage and health system

    UFUNCTION()
    void OnHealthChanged(UCSHealthComponent* OwningHealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    //////////////////////////////////////////////////////////////////////////
    // Replication

    /** Update hud when current weapon has changed */
    UFUNCTION()
    void OnRep_CurrentWeapon();

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCSHealthComponent* HealthComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess="true"))
    UAbilitySystemComponent* AbilitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess="true"))
    UCSAttributeSet*	AttributeSet;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float ZoomedFOV;

    UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0f))
    float ZoomInterpSpeed;

    float DefaultFOV;

    /** Check if character was aiming before start reloading */
    bool bWasAiming;

    /** Is Aiming Down Sights? */
    UPROPERTY(Transient, Replicated)
    bool bAiming;

    UPROPERTY(VisibleDefaultsOnly, Category = "Player")
    FName WeaponAttachSocketName;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
    ACSWeapon* CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    TSubclassOf<ACSWeapon> StarterWeaponClass;

    UPROPERTY(Replicated)
    bool bDied;

    /** Material instances for setting team color on mesh */
    UPROPERTY(Transient)
    TArray<class UMaterialInstanceDynamic*> MeshMIDs;

public:

    /** Event raised on aiming is changed */
    UPROPERTY(BlueprintAssignable, Category = "Player")
    FCharacterAimStateChangedSignature OnAimStateChange;

    /** Event raised when character equips new weapon */
    UPROPERTY(BlueprintAssignable, Category = "Player")
    FCharactereEquipWeaponSignature OnWeaponEquip;

    UFUNCTION(BlueprintCallable, Category = "Player | Abilities")
    void AcquireAbility(TSubclassOf<UGameplayAbility> AbiltyToAcquire);

    //////////////////////////////////////////////////////////////////////////
    // Statistics

    void RegisterAction(ECharacterAction Action, float Amount = 0.0f);

    //////////////////////////////////////////////////////////////////////////
    // Materials

    /** Handle mesh colors on specified material instance */
    void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

    /** Update the team color of all player meshes. */
    void UpdateTeamColorsAllMIDs();

public:

    //////////////////////////////////////////////////////////////////////////
    // Reading data

    // Current equipped weapon
    UFUNCTION(BlueprintCallable, Category = "Character")
    ACSWeapon* GetCurrentWeapon() const;

    // Get health component
    UFUNCTION(BlueprintCallable, Category = "Character")
    UCSHealthComponent* GetHealthComponent() const;

    /** Check if pawn is aiming down sights */
    UFUNCTION(BlueprintCallable, Category = "Character")
    bool IsAiming() const;

    /** Check if character is alive */
    UFUNCTION(BlueprintCallable, Category = "Character")
    bool IsAlive() const;

    /** Get Weapon Socket Name */
    FName GetWeaponAttachPoint() const;
};

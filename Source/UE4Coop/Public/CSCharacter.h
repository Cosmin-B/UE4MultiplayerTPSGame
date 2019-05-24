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
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
    UseAbility1 UMETA(DisplayName = "Use Spell 1"),
    UseAbility2 UMETA(DisplayName = "Use Spell 2"),
    UseAbility3 UMETA(DisplayName = "Use Spell 3"),
    UseAbility4 UMETA(DisplayName = "Use Spell 4"),
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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    void MoveForward(float Value);

    void MoveRight(float Value);

    void MoveJump();

    void BeginCrouch();

    void EndCrouch();

    void BeginZoom();

    void EndZoom();

    UFUNCTION()
    void OnHealthChanged(UCSHealthComponent* OwningHealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void PossessedBy(AController* NewController) override;

    virtual FVector GetPawnViewLocation() const override;
protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCSHealthComponent* HealthComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess="true"))
    UAbilitySystemComponent* AbilitySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
    TSubclassOf<UGameplayAbility> Ability;

    bool bWantsToZoom;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    float ZoomedFOV;

    UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0f))
    float ZoomInterpSpeed;

    float DefaultFOV;

    UPROPERTY(VisibleDefaultsOnly, Category = "Player")
    FName WeaponAttachSocketName;

    UPROPERTY(Replicated)
    ACSWeapon* CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "Player")
    TSubclassOf<ACSWeapon> StarterWeaponClass;

    UPROPERTY(Replicated)
    bool bDied;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Player")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void StopFire();

};

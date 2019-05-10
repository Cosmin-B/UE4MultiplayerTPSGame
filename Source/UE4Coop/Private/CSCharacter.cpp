// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/CSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ACSCharacter::ACSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->SetupAttachment(RootComponent);

    GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp);

    ZoomedFOV = 65.5f;
    ZoomInterpSpeed = 20.0f;
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();

    DefaultFOV = CameraComp->FieldOfView;
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
    float NewFOV    = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

    CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
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

FVector ACSCharacter::GetPawnViewLocation() const
{
    if (CameraComp)
        return CameraComp->GetComponentLocation();

    return Super::GetPawnViewLocation();
}

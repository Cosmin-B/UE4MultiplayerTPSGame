// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSPowerUpSpawner.generated.h"

class UDecalComponent;
class USphereComponent;
class ACSPowerUpBase;

UCLASS()
class UE4COOP_API ACSPowerUpSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSPowerUpSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    void Respawn();

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UDecalComponent* DecalComp;

    UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    USphereComponent* SphereComp;

    UPROPERTY(EditInstanceOnly, Category = "PowerUp")
    TSubclassOf<ACSPowerUpBase> PowerUpClass;

    UPROPERTY(EditInstanceOnly, Category = "PowerUp")
    float RespawnTimer;

    FTimerHandle TimerHandle_RespawnTimer;

    ACSPowerUpBase* PowerUpInstance;

public:

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

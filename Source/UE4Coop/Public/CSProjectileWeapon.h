// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSWeapon.h"
#include "CSProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UE4COOP_API ACSProjectileWeapon : public ACSWeapon
{
	GENERATED_BODY()

protected:
    virtual void Fire() override;

    UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
    TSubclassOf<AActor> ProjectileClass;
};
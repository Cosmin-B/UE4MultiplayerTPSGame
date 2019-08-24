// Fill out your copyright notice in the Description page of Project Settings.


#include "CSAIController.h"
#include "CSWeapon.h"
#include "CSAdvancedAI.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

ACSAIController::ACSAIController() : AAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoardComp"));

    BrainComponent = BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    bWantsPlayerState = true;
}

void ACSAIController::GameHasEnded(class AActor* EndGameFocus /*= NULL*/, bool bIsWinner /*= false*/)
{
    // Stop the behaviour tree/logic
    BehaviorComp->StopTree();

    // Stop any movement we already have
    StopMovement();

    // Finally stop firing
    ACSCharacter* MyAI = Cast<ACSAdvancedAI>(GetPawn());
    ACSWeapon* MyWeapon = MyAI ? MyAI->GetCurrentWeapon() : nullptr;

    if (MyWeapon == NULL)
        return;

    MyWeapon->StopFire();
}

void ACSAIController::OnPossess(class APawn* InPawn)
{
    ACSAdvancedAI* MyAI = Cast<ACSAdvancedAI>(InPawn);

    // Start behavior
    if (MyAI && MyAI->AIBehavior)
    {
        if (MyAI->AIBehavior->BlackboardAsset)
            BlackboardComp->InitializeBlackboard(*MyAI->AIBehavior->BlackboardAsset);

        NeedAmmoKeyID = BlackboardComp->GetKeyID("NeedAmmo");

        BehaviorComp->StartTree(*(MyAI->AIBehavior));
    }
}

void ACSAIController::OnUnPossess()
{
    Super::OnUnPossess();

    BehaviorComp->StopTree();
}

void ACSAIController::CheckAmmo(const class ACSWeapon* CurrentWeapon)
{
    if (CurrentWeapon && BlackboardComp)
    {
        const int32 Ammo = CurrentWeapon->GetCurrentAmmo();
        const int32 MaxAmmo = CurrentWeapon->GetMaxAmmo();
        const float Ratio = (float)Ammo / (float)MaxAmmo;

        BlackboardComp->SetValue<UBlackboardKeyType_Bool>(NeedAmmoKeyID, (Ratio <= 0.1f));
    }
}
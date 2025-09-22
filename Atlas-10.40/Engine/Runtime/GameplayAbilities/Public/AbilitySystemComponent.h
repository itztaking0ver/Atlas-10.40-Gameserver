#pragma once
#include "../Runtime.h"

namespace AbilitySystemComponent
{
    static void InternalServerTryActivateAbility(UFortAbilitySystemComponentAthena*, FGameplayAbilitySpecHandle, bool, FPredictionKey&, FGameplayEventData*);
    void GiveAbilitySet(UFortAbilitySystemComponent* AbilitySystemComponent, UFortAbilitySet* Set);

    void Patch();
}
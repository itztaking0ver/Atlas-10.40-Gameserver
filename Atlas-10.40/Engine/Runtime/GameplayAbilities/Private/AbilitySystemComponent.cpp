#include "../Public/AbilitySystemComponent.h"

void AbilitySystemComponent::InternalServerTryActivateAbility(
    UFortAbilitySystemComponentAthena* AbilitySystemComponent,
    FGameplayAbilitySpecHandle Handle,
    bool InputPressed,
    FPredictionKey& PredictionKey,
    FGameplayEventData* TriggerEventData)
{
    auto Spec = AbilitySystemComponent->ActivatableAbilities.Items.Search(
        [&](FGameplayAbilitySpec& Item) { return Item.Handle.Handle == Handle.Handle; }
    );

    if (!Spec)
    {
        return AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
    }

    Spec->InputPressed = InputPressed;
    UGameplayAbility* InstancedAbility = nullptr;

    using TryActivateFunc = bool (*)(
        UAbilitySystemComponent*,
        FGameplayAbilitySpecHandle,
        FPredictionKey,
        UGameplayAbility**,
        void*,
        const FGameplayEventData*
        );

    auto InternalTryActivate = reinterpret_cast<TryActivateFunc>(Runtime::Offsets::InternalTryActivateAbility);

    if (!InternalTryActivate(
        AbilitySystemComponent,
        Handle,
        PredictionKey,
        &InstancedAbility,
        nullptr,
        TriggerEventData))
    {
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
    }

    AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
}

void AbilitySystemComponent::GiveAbilitySet(UFortAbilitySystemComponent* AbilitySystemComponent, UFortAbilitySet* Set)
{
    if (Set)
    {
        for (auto& GameplayAbility : Set->GameplayAbilities)
        {
            if (!AbilitySystemComponent || !GameplayAbility->DefaultObject)
                return;

            FGameplayAbilitySpec Spec{};
            ((void (*)(FGameplayAbilitySpec*, UGameplayAbility*, int, int, UObject*))Runtime::Offsets::ConstructAbilitySpec)(&Spec, (UGameplayAbility*)GameplayAbility->DefaultObject, 1, -1, nullptr);
            ((FGameplayAbilitySpecHandle * (__fastcall*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec)) Runtime::Offsets::InternalGiveAbility)(AbilitySystemComponent, &Spec.Handle, Spec);
        }
        for (auto& GameplayEffect : Set->PassiveGameplayEffects)
            AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GameplayEffect.GameplayEffect.Get(), GameplayEffect.Level, AbilitySystemComponent->MakeEffectContext());
    }
}

void AbilitySystemComponent::Patch() {
    Runtime::Every<UAbilitySystemComponent>(Runtime::Offsets::InternalServerTryActivateAbilityVft, InternalServerTryActivateAbility);
}
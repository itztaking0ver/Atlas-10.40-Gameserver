#include "../Public/Actor.h"

int Actor::GetNetMode(AActor*) 
{
    return 1;
}

void Actor::Patch()
{
    Runtime::Hook(Runtime::Offsets::WorldNetMode, Actor::GetNetMode);
}
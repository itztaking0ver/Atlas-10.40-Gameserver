#pragma once
#include "../Runtime.h"

namespace FortCreativeMoveTool
{
    static void ServerSetAllowGravity(AFortCreativeMoveTool* Tool, bool bAllow);
    static void ComputeSelectionSetTransformAndBounds(AFortCreativeMoveTool* Tool, FTransform& OutTransform, FBox& OutBounds);
    static void ServerStartInteracting(AFortCreativeMoveTool* Tool, TArray<AActor*>& Actors, FTransform DragStart);
    static void ServerDuplicateStartInteracting(UObject* Context, FFrame& Stack);
    static void ServerSpawnActorWithTransform(UObject* Context, FFrame& Stack);

    void Patch();
}
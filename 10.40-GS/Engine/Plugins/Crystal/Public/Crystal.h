#pragma once
#include "../framework.h"

#ifdef Crystal
#undef Crystal
#endif

class Crystal
{
public:
    bool bCreative = false;
    bool bLategame = false;

    inline static std::string PlaylistID = "/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2";
public:
    void SetState(const std::string& State);
    void Initialize();
};

inline Crystal* UCrystal = NULL;

#pragma once
#include "../framework.h"

#ifdef Atlas
#undef Atlas
#endif

class Atlas
{
public:
    bool bCreative = false;
    bool bLategame = false;

    inline static std::string PlaylistID = "/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2";
public:
    void SetState(const std::string& State);
    void Initialize();
};

inline Atlas* UAtlas = NULL;

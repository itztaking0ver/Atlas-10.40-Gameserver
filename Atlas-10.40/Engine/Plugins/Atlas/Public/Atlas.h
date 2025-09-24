#pragma once
#include "../framework.h"

#ifdef Atlas
#undef Atlas
#endif

class Atlas
{
public:
    // Arena and Tournament Configuration
    static inline bool bArena = false;
    static inline bool bTournament = false;
    static inline int RefreshMatsOnPawn = 50;
    bool bCreative = false;
    bool bLategame = false;

    inline static std::string PlaylistID = 
            //"/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2"
            //"/Game/Athena/Playlists/Showdown/Playlist_ShowdownAlt_Solo.Playlist_ShowdownAlt_Solo";
            //"/Game/Athena/Playlists/Showdown/Tournament/Playlist_ShowdownTournament_Solo.Playlist_ShowdownTournament_Solo";
            "/Game/Athena/Playlists/Fill/Playlist_Fill_Solo.Playlist_Fill_Solo";
            //"/Game/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo";
            //"/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground";
            // "/Game/Athena/Playlists/Low/Playlist_Low_Solo.Playlist_Low_Solo";
            
    // Bot Configuration
    bool bBotsEnabled = true;
    int MaxBotsToSpawn = 95;
    int MinPlayersForEarlyStart = 95;

    // Team Management
    inline static int NextTeamIndex = 0;
    inline static int CurrentPlayersOnTeam = 0;
    inline static int MaxPlayersPerTeam = 1;

    // Bot Team Management
    inline static int NextBotTeamIndex = 0;
    inline static int CurrentBotsOnTeam = 0;

    // Vehicle Configuration (only works when lategame is off)
    bool bVehiclesEnabled = false;

    // Backend Configuration
    static inline bool PlayerRewards = true;
    static inline std::string BackendIP = "http://127.0.0.1:3551/";
    static inline std::string BackendAPIKey = "AtlasAPI6767";
    


public:
    void SetState(const std::string& State);
    void Initialize();
};

inline Atlas* UAtlas = NULL;

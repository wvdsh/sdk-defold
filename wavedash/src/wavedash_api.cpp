#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_HTML5)

typedef void (*OnEventCallback)(const char* event, const char* payload, uint32_t payload_length);

extern "C" {
    void WavedashJs_Init(OnEventCallback callback);
    void WavedashJs_CreateLobby(uint32_t visibility, uint32_t maxPlayers);
    void WavedashJs_JoinLobby(uint32_t lobbyId);
}

static dmScript::LuaCallbackInfo* g_EventCallback = 0x0;

static void Wavedash_OnEventCallback(const char* event, const char* payload, uint32_t payload_length)
{
    if (!dmScript::IsCallbackValid(g_EventCallback))
    {
        dmLogError("Event callback is invalid");
        return;
    }

    lua_State* L = dmScript::GetCallbackLuaContext(g_EventCallback);

    DM_LUA_STACK_CHECK(L, 0);

    if (!dmScript::SetupCallback(g_EventCallback))
    {
        dmLogError("Unable to set up event callback");
        return;
    }

    lua_pushstring(L, event);
    dmScript::JsonToLua(L, payload, payload_length);
    int ret = dmScript::PCall(L, 3, 0);
    (void)ret;

    dmScript::TeardownCallback(g_EventCallback);
}

/**
 * Initialize Wavedash SDK
 * @name init
 * @function listener Event callback function
 */
int Wavedash_Init(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    g_EventCallback = dmScript::CreateCallback(L, 1);
    WavedashJs_Init(Wavedash_OnEventCallback);
    return 0;
}

/**
 * Create lobby
 * @name create_lobby
 * @number visibility
 * @number max_players
 */
int Wavedash_CreateLobby(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    uint32_t visibility = luaL_checknumber(L, 1);
    uint32_t maxPlayers = luaL_checknumber(L, 2);

    WavedashJs_CreateLobby(visibility, maxPlayers);
    return 0;
}

/**
 * Join lobby
 * @name join_lobby
 * @number lobby_id
 */
int Wavedash_JoinLobby(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    uint32_t lobbyId = luaL_checknumber(L, 1);

    WavedashJs_JoinLobby(lobbyId);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"init", Wavedash_Init},
    {"create_lobby", Wavedash_CreateLobby},
    {"join_lobby", Wavedash_JoinLobby},
    {0, 0}
};

#define SETCONSTANT_STRING(name, value) \
    lua_pushstring(L, (const char*) (value)); \
    lua_setfield(L, -2, #name);\

#define SETCONSTANT_NUMBER(name, value) \
    lua_pushnumber(L, (lua_Number) (value)); \
    lua_setfield(L, -2, #name);\

void WavedashLuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    luaL_register(L, "wavedash", Module_methods);

    /**
     * EVENT_LOBBY_MESSAGE
     * @field EVENT_LOBBY_MESSAGE
     */
    SETCONSTANT_STRING(EVENT_LOBBY_MESSAGE, "LobbyMessage");
    /**
     * EVENT_LOBBY_JOINED
     * @field EVENT_LOBBY_JOINED
     */
    SETCONSTANT_STRING(EVENT_LOBBY_JOINED, "LobbyJoined")
    /**
     * EVENT_LOBBY_KICKED
     * @field EVENT_LOBBY_KICKED
     */
    SETCONSTANT_STRING(EVENT_LOBBY_KICKED, "LobbyKicked")
    /**
     * EVENT_LOBBY_USERS_UPDATED
     * @field EVENT_LOBBY_USERS_UPDATED
     */
    SETCONSTANT_STRING(EVENT_LOBBY_USERS_UPDATED, "LobbyUsersUpdated")
    /**
     * EVENT_LOBBY_DATA_UPDATED
     * @field EVENT_LOBBY_DATA_UPDATED
     */
    SETCONSTANT_STRING(EVENT_LOBBY_DATA_UPDATED, "LobbyDataUpdated")
    /**
     * EVENT_LOBBY_INVITE
     * @field EVENT_LOBBY_INVITE
     */
    SETCONSTANT_STRING(EVENT_LOBBY_INVITE, "LobbyInvite")
    /**
     * EVENT_P2P_CONNECTION_ESTABLISHED
     * @field EVENT_P2P_CONNECTION_ESTABLISHED
     */
    SETCONSTANT_STRING(EVENT_P2P_CONNECTION_ESTABLISHED, "P2PConnectionEstablished")
    /**
     * EVENT_P2P_CONNECTION_FAILED
     * @field EVENT_P2P_CONNECTION_FAILED
     */
    SETCONSTANT_STRING(EVENT_P2P_CONNECTION_FAILED, "P2PConnectionFailed")
    /**
     * EVENT_P2P_PEER_DISCONNECTED
     * @field EVENT_P2P_PEER_DISCONNECTED
     */
    SETCONSTANT_STRING(EVENT_P2P_PEER_DISCONNECTED, "P2PPeerDisconnected")
    /**
     * EVENT_P2P_PEER_RECONNECTING
     * @field EVENT_P2P_PEER_RECONNECTING
     */
    SETCONSTANT_STRING(EVENT_P2P_PEER_RECONNECTING, "P2PPeerReconnecting")
    /**
     * EVENT_P2P_PEER_RECONNECTED
     * @field EVENT_P2P_PEER_RECONNECTED
     */
    SETCONSTANT_STRING(EVENT_P2P_PEER_RECONNECTED, "P2PPeerReconnected")
    /**
     * EVENT_P2P_PACKET_DROPPED
     * @field EVENT_P2P_PACKET_DROPPED
     */
    SETCONSTANT_STRING(EVENT_P2P_PACKET_DROPPED, "P2PPacketDropped")
    /**
     * EVENT_STATS_STORED
     * @field EVENT_STATS_STORED
     */
    SETCONSTANT_STRING(EVENT_STATS_STORED, "StatsStored")
    /**
     * EVENT_BACKEND_CONNECTED
     * @field EVENT_BACKEND_CONNECTED
     */
    SETCONSTANT_STRING(EVENT_BACKEND_CONNECTED, "BackendConnected")
    /**
     * EVENT_BACKEND_DISCONNECTED
     * @field EVENT_BACKEND_DISCONNECTED
     */
    SETCONSTANT_STRING(EVENT_BACKEND_DISCONNECTED, "BackendDisconnected")
    /**
     * EVENT_BACKEND_RECONNECTING
     * @field EVENT_BACKEND_RECONNECTING
     */
    SETCONSTANT_STRING(EVENT_BACKEND_RECONNECTING, "BackendReconnecting")
    /**
     * EVENT_FULLSCREEN_CHANGED
     * @field EVENT_FULLSCREEN_CHANGED
     */
    SETCONSTANT_STRING(EVENT_FULLSCREEN_CHANGED, "FullscreenChanged")
    /**
     * LOBBY_VISIBILITY_PUBLIC
     * @field LOBBY_VISIBILITY_PUBLIC
     */
    SETCONSTANT_NUMBER(LOBBY_VISIBILITY_PUBLIC, 0)
    /**
     * LOBBY_VISIBILITY_FRIENDS_ONLY
     * @field LOBBY_VISIBILITY_FRIENDS_ONLY
     */
    SETCONSTANT_NUMBER(LOBBY_VISIBILITY_FRIENDS_ONLY, 1)
    /**
     * LOBBY_VISIBILITY_PRIVATE
     * @field LOBBY_VISIBILITY_PRIVATE
     */
    SETCONSTANT_NUMBER(LOBBY_VISIBILITY_PRIVATE, 2)
    /**
     * LEADERBOARD_SORT_ORDER_ASC
     * @field LEADERBOARD_SORT_ORDER_ASC
     */
    SETCONSTANT_NUMBER(LEADERBOARD_SORT_ORDER_ASC, 0)
    /**
     * LEADERBOARD_SORT_ORDER_DESC
     * @field LEADERBOARD_SORT_ORDER_DESC
     */
    SETCONSTANT_NUMBER(LEADERBOARD_SORT_ORDER_DESC, 1)
    /**
     * LEADERBOARD_DISPLAY_TYPENUMERIC
     * @field LEADERBOARD_DISPLAY_TYPENUMERIC
     */
    SETCONSTANT_NUMBER(LEADERBOARD_DISPLAY_TYPENUMERIC, 0)
    /**
     * LEADERBOARD_DISPLAY_TYPETIME_SECONDS
     * @field LEADERBOARD_DISPLAY_TYPETIME_SECONDS
     */
    SETCONSTANT_NUMBER(LEADERBOARD_DISPLAY_TYPETIME_SECONDS, 1)
    /**
     * LEADERBOARD_DISPLAY_TYPETIME_MILLISECONDS
     * @field LEADERBOARD_DISPLAY_TYPETIME_MILLISECONDS
     */
    SETCONSTANT_NUMBER(LEADERBOARD_DISPLAY_TYPETIME_MILLISECONDS, 2)
    /**
     * LEADERBOARD_DISPLAY_TYPETIME_GAME_TICKS
     * @field LEADERBOARD_DISPLAY_TYPETIME_GAME_TICKS
     */
    SETCONSTANT_NUMBER(LEADERBOARD_DISPLAY_TYPETIME_GAME_TICKS, 3)
    /**
     * UGC_TYPE_SCREENSHOT
     * @field UGC_TYPE_SCREENSHOT
     */
    SETCONSTANT_NUMBER(UGC_TYPE_SCREENSHOT, 0)
    /**
     * UGC_TYPE_VIDEO
     * @field UGC_TYPE_VIDEO
     */
    SETCONSTANT_NUMBER(UGC_TYPE_VIDEO, 1)
    /**
     * UGC_TYPE_COMMUNITY
     * @field UGC_TYPE_COMMUNITY
     */
    SETCONSTANT_NUMBER(UGC_TYPE_COMMUNITY, 2)
    /**
     * UGC_TYPE_GAME_MANAGED
     * @field UGC_TYPE_GAME_MANAGED
     */
    SETCONSTANT_NUMBER(UGC_TYPE_GAME_MANAGED, 3)
    /**
     * UGC_TYPE_OTHER
     * @field UGC_TYPE_OTHER
     */
    SETCONSTANT_NUMBER(UGC_TYPE_OTHER, 4)
    /**
     * UGC_VISIBILITY_PUBLIC
     * @field UGC_VISIBILITY_PUBLIC
     */
    SETCONSTANT_NUMBER(UGC_VISIBILITY_PUBLIC, 0)
    /**
     * UGC_VISIBILITY_FRIENDS_ONLY
     * @field UGC_VISIBILITY_FRIENDS_ONLY
     */
    SETCONSTANT_NUMBER(UGC_VISIBILITY_FRIENDS_ONLY, 1)
    /**
     * UGC_VISIBILITY_PRIVATE
     * @field UGC_VISIBILITY_PRIVATE
     */
    SETCONSTANT_NUMBER(UGC_VISIBILITY_PRIVATE, 2)
    /**
     * AVATAR_SIZE_SMALL
     * @field AVATAR_SIZE_SMALL
     */
    SETCONSTANT_NUMBER(AVATAR_SIZE_SMALL, 64)
    /**
     * AVATAR_SIZE_MEDIUM
     * @field AVATAR_SIZE_MEDIUM
     */
    SETCONSTANT_NUMBER(AVATAR_SIZE_MEDIUM, 128)
    /**
     * AVATAR_SIZE_LARGE
     * @field AVATAR_SIZE_LARGE
     */
    SETCONSTANT_NUMBER(AVATAR_SIZE_LARGE, 256)
    /**
     * LOBBY_KICKED_REASON_KICKED
     * @field LOBBY_KICKED_REASON_KICKED
     */
    SETCONSTANT_STRING(LOBBY_KICKED_REASON_KICKED, "KICKED")
    /**
     * LOBBY_KICKED_REASON_ERROR
     * @field LOBBY_KICKED_REASON_ERROR
     */
    SETCONSTANT_STRING(LOBBY_KICKED_REASON_ERROR, "ERROR")
    /**
     * LOBBY_USER_CHANGE_TYPE_JOINED
     * @field LOBBY_USER_CHANGE_TYPE_JOINED
     */
    SETCONSTANT_STRING(LOBBY_USER_CHANGE_TYPE_JOINED, "JOINED")
    /**
     * LOBBY_USER_CHANGE_TYPE_LEFT
     * @field LOBBY_USER_CHANGE_TYPE_LEFT
     */
    SETCONSTANT_STRING(LOBBY_USER_CHANGE_TYPE_LEFT, "LEFT")
    /**
     * P2P_PACKET_DROP_REASON_QUEUE_FULL
     * @field P2P_PACKET_DROP_REASON_QUEUE_FULL
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_QUEUE_FULL, "QUEUE_FULL")
    /**
     * P2P_PACKET_DROP_REASON_PAYLOAD_TOO_LARGE
     * @field P2P_PACKET_DROP_REASON_PAYLOAD_TOO_LARGE
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_PAYLOAD_TOO_LARGE, "PAYLOAD_TOO_LARGE")
    /**
     * P2P_PACKET_DROP_REASON_INVALID_PAYLOAD_SIZE
     * @field P2P_PACKET_DROP_REASON_INVALID_PAYLOAD_SIZE
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_INVALID_PAYLOAD_SIZE, "INVALID_PAYLOAD_SIZE")
    /**
     * P2P_PACKET_DROP_REASON_INVALID_CHANNEL
     * @field P2P_PACKET_DROP_REASON_INVALID_CHANNEL
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_INVALID_CHANNEL, "INVALID_CHANNEL")
    /**
     * P2P_PACKET_DROP_REASON_MALFORMED
     * @field P2P_PACKET_DROP_REASON_MALFORMED
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_MALFORMED, "MALFORMED")
    /**
     * P2P_PACKET_DROP_REASON_PEER_NOT_READY
     * @field P2P_PACKET_DROP_REASON_PEER_NOT_READY
     */
    SETCONSTANT_STRING(P2P_PACKET_DROP_REASON_PEER_NOT_READY, "PEER_NOT_READY")

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

#undef SETCONSTANT_STRING
#undef SETCONSTANT_NUMBER

#endif
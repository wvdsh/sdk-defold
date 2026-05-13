#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_HTML5)

#include <cmath>
#include <cstring>
#include <string>

/**
 * Wavedash Defold API
 * @namespace wavedash
 * @document
 */

typedef void (*OnEventCallback)(const char* event, const char* payload, uint32_t payload_length);

extern "C" {
    int WavedashJs_Init(OnEventCallback callback);
    void WavedashJs_ReadyForEvents();
    void WavedashJs_UpdateLoadProgressZeroToOne(double progress);
    void WavedashJs_LoadComplete();
    void WavedashJs_ToggleOverlay();
    int WavedashJs_IsFullscreen();
    void WavedashJs_RequestFullscreenAsync(int fullscreen);
    void WavedashJs_ToggleFullscreenAsync();

    const char* WavedashJs_GetUser();
    const char* WavedashJs_GetUsername(double user_id);
    double WavedashJs_GetUserId();
    void WavedashJs_GetUserJwtAsync();
    const char* WavedashJs_GetLaunchParams();
    void WavedashJs_ListFriendsAsync();
    const char* WavedashJs_GetUserAvatarUrl(double user_id, double size);

    void WavedashJs_GetLeaderboardAsync(const char* name);
    void WavedashJs_GetOrCreateLeaderboardAsync(const char* name, double sort_order, double display_type);
    double WavedashJs_GetLeaderboardEntryCount(double leaderboard_id);
    void WavedashJs_GetMyLeaderboardEntriesAsync(double leaderboard_id);
    void WavedashJs_ListLeaderboardEntriesAroundUserAsync(double leaderboard_id, double count_ahead, double count_behind, int friends_only);
    void WavedashJs_ListLeaderboardEntriesAsync(double leaderboard_id, double offset, double limit, int friends_only);
    void WavedashJs_UploadLeaderboardScoreAsync(double leaderboard_id, double score, int keep_best, double ugc_id);

    void WavedashJs_CreateUGCItemAsync(double ugc_type, const char* title, const char* description, double visibility, const char* file_path);
    void WavedashJs_UpdateUGCItemAsync(double ugc_id, const char* title, const char* description, double visibility, const char* file_path);
    void WavedashJs_DownloadUGCItemAsync(double ugc_id, const char* file_path);
    void WavedashJs_DeleteRemoteFileAsync(const char* file_path);
    void WavedashJs_DownloadRemoteFileAsync(const char* file_path);
    void WavedashJs_UploadRemoteFileAsync(const char* file_path);
    void WavedashJs_ListRemoteDirectoryAsync(const char* path);
    void WavedashJs_DownloadRemoteDirectoryAsync(const char* path);
    void WavedashJs_WriteLocalFileAsync(const char* file_path, const void* data, uint32_t data_length);
    const char* WavedashJs_ReadLocalFileAsync(const char* file_path);

    int WavedashJs_GetAchievement(const char* identifier);
    double WavedashJs_GetStat(const char* identifier);
    int WavedashJs_SetAchievement(const char* identifier, int store_now);
    int WavedashJs_SetStat(const char* identifier, double value, int store_now);
    void WavedashJs_RequestStatsAsync();
    int WavedashJs_StoreStats();

    double WavedashJs_GetP2PMaxPayloadSize();
    double WavedashJs_GetP2PMaxIncomingMessages();
    const char* WavedashJs_GetP2POutgoingMessageBuffer(uint32_t* out_length);
    int WavedashJs_SendP2PMessage(double to_user_id, double app_channel, int reliable, const void* payload, uint32_t payload_length, double payload_size);
    int WavedashJs_BroadcastP2PMessage(double app_channel, int reliable, const void* payload, uint32_t payload_length, double payload_size);
    const char* WavedashJs_ReadP2PMessageFromChannel(double app_channel);
    const char* WavedashJs_DrainP2PChannelToBuffer(double app_channel, uint32_t* out_length);

    void WavedashJs_CreateLobbyAsync(double visibility, double max_players);
    void WavedashJs_JoinLobbyAsync(double lobby_id);
    void WavedashJs_ListAvailableLobbiesAsync(int friends_only);
    const char* WavedashJs_GetLobbyUsers(double lobby_id);
    double WavedashJs_GetNumLobbyUsers(double lobby_id);
    const char* WavedashJs_GetLobbyHostId(double lobby_id);
    const char* WavedashJs_GetLobbyData(double lobby_id, const char* key);
    int WavedashJs_SetLobbyData(double lobby_id, const char* key, const char* value_json);
    int WavedashJs_DeleteLobbyData(double lobby_id, const char* key);
    void WavedashJs_LeaveLobbyAsync(double lobby_id);
    int WavedashJs_SendLobbyMessage(double lobby_id, const char* message);
    void WavedashJs_InviteUserToLobbyAsync(double lobby_id, double user_id);
    void WavedashJs_GetLobbyInviteLinkAsync(int copy_to_clipboard);
    void WavedashJs_UpdateUserPresenceAsync(const char* data_json);
    void WavedashJs_EnsureGameplayJwtAsync();

    void WavedashJs_Free(void* ptr);
}

static dmScript::LuaCallbackInfo*   g_EventCallback = 0x0;
static lua_State*                   g_AsyncThread = 0x0;
static char*                        g_AsyncEventId = 0;


static void DumpStack(lua_State* L)
{
    dmLogInfo("Stack:");
    for (int i = 1; i <= lua_gettop(L); i++)
    {
        int type = lua_type(L, i);
        const char* name = lua_typename(L, type);
        switch (type)
        {
            case LUA_TNIL:
            case LUA_TTABLE:
            case LUA_TFUNCTION:
            case LUA_TUSERDATA:
            case LUA_TTHREAD:
            case LUA_TLIGHTUSERDATA:
            default:
                dmLogInfo("%d = %s", i, name);
                break;
            case LUA_TNUMBER:
                dmLogInfo("%d = %f (%s)", i, luaL_checknumber(L, i), name);
                break;
            case LUA_TBOOLEAN:
                dmLogInfo("%d = %d (%s)", i, lua_toboolean(L, i), name);
                break;
            case LUA_TSTRING:
                dmLogInfo("%d = %s (%s)", i, luaL_checkstring(L, i), name);
                break;
        }
    }
}

static int AwaitAsyncEvent(lua_State* L, char* eventId)
{
    int is_main = lua_pushthread(L);
    lua_pop(L, 1);
    if (is_main)
    {
        g_AsyncThread = 0;
        g_AsyncEventId = 0x0;
        return 0;
    }

    g_AsyncThread = L;
    g_AsyncEventId = eventId;
    return lua_yield(L, 0);
}


static void Wavedash_OnEventCallback(const char* event, const char* payload, uint32_t payload_length)
{
    if (g_AsyncThread && (strcmp(g_AsyncEventId, event) == 0))
    {
        lua_State* L = g_AsyncThread;
        g_AsyncThread = 0x0;
        g_AsyncEventId = 0x0;
        dmScript::JsonToLua(L, payload, payload_length);
        int res = lua_resume(L, 1);
        if ((res != LUA_YIELD) && (res != 0))
        {
            dmLogError("Coroutine resumed with error %d", res)
        }
        return;
    }

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

static double OptionalNumberArg(lua_State* L, int index)
{
    return lua_isnoneornil(L, index) ? NAN : luaL_checknumber(L, index);
}

static int OptionalBoolArg(lua_State* L, int index)
{
    if (lua_isnoneornil(L, index))
    {
        return -1;
    }
    return lua_toboolean(L, index) ? 1 : 0;
}

static const char* OptionalStringArg(lua_State* L, int index)
{
    return lua_isnoneornil(L, index) ? 0 : luaL_checkstring(L, index);
}

static void PushAndFreeString(lua_State* L, const char* value)
{
    if (!value)
    {
        lua_pushnil(L);
        return;
    }

    lua_pushstring(L, value);
    WavedashJs_Free((void*) value);
}

static void PushAndFreeJson(lua_State* L, const char* value)
{
    if (!value)
    {
        lua_pushnil(L);
        return;
    }

    dmScript::JsonToLua(L, value, (uint32_t) strlen(value));
    WavedashJs_Free((void*) value);
}

static void PushAndFreeBytes(lua_State* L, const char* value, uint32_t length)
{
    if (!value)
    {
        lua_pushnil(L);
        return;
    }

    lua_pushlstring(L, value, length);
    WavedashJs_Free((void*) value);
}

static std::string EscapeJsonString(const char* value)
{
    std::string result;
    for (const char* p = value; *p; ++p)
    {
        switch (*p)
        {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += *p;
                break;
        }
    }
    return result;
}

static bool LuaValueToJsonLiteral(lua_State* L, int index, std::string& json)
{
    int type = lua_type(L, index);
    switch (type)
    {
        case LUA_TNIL:
            json = "null";
            return true;
        case LUA_TBOOLEAN:
            json = lua_toboolean(L, index) ? "true" : "false";
            return true;
        case LUA_TNUMBER:
            json = std::to_string(lua_tonumber(L, index));
            return true;
        case LUA_TSTRING:
            json = "\"";
            json += EscapeJsonString(lua_tostring(L, index));
            json += "\"";
            return true;
        default:
            dmLogError("Unsupported Lua value for JSON literal at argument %d", index);
            return false;
    }
}

static const char* RawJsonStringArg(lua_State* L, int index)
{
    if (lua_isnoneornil(L, index))
    {
        return 0;
    }

    if (!lua_isstring(L, index))
    {
        dmLogError("Expected JSON string at argument %d", index);
        return 0;
    }

    return lua_tostring(L, index);
}

/**
 * Initialize Wavedash.
 * @name init
 * @function callback
 */
int Wavedash_Init(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    g_EventCallback = dmScript::CreateCallback(L, 1);
    lua_pushboolean(L, WavedashJs_Init(Wavedash_OnEventCallback));
    return 1;
}

/**
 * Signal that the game is ready to receive events.
 * @name ready_for_events
 */
int Wavedash_ReadyForEvents(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    WavedashJs_ReadyForEvents();
    return 0;
}

/**
 * Update the load progress.
 * @name update_load_progress_zero_to_one
 * @number progress
 */
int Wavedash_UpdateLoadProgressZeroToOne(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    WavedashJs_UpdateLoadProgressZeroToOne(luaL_checknumber(L, 1));
    return 0;
}

/**
 * Signal that loading is complete.
 * @name load_complete
 */
int Wavedash_LoadComplete(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    WavedashJs_LoadComplete();
    return 0;
}

/**
 * Toggle the overlay.
 * @name toggle_overlay
 */
int Wavedash_ToggleOverlay(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    WavedashJs_ToggleOverlay();
    return 0;
}

/**
 * Check if fullscreen is enabled.
 * @name is_fullscreen
 */
int Wavedash_IsFullscreen(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_IsFullscreen());
    return 1;
}

/**
 * Request fullscreen mode.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'requestFullscreen' or as a return value if the function is called from
 * a coroutine.
 * @name request_fullscreen_async
 * @boolean fullscreen
 */
int Wavedash_RequestFullscreenAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_RequestFullscreenAsync(lua_toboolean(L, 1) ? 1 : 0);
    }
    return AwaitAsyncEvent(L, "requestFullscreen");
}

/**
 * Toggle fullscreen mode.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'toggleFullscreen' or as a return value if the function is called from
 * a coroutine.
 * @name toggle_fullscreen_async
 */
int Wavedash_ToggleFullscreenAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ToggleFullscreenAsync();
    }
    return AwaitAsyncEvent(L, "toggleFullscreen");
}

/**
 * Get the current user.
 * @name get_user
 */
int Wavedash_GetUser(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_GetUser());
    return 1;
}

/**
 * Get a username.
 * @name get_username
 * @number user_id?
 */
int Wavedash_GetUsername(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeString(L, WavedashJs_GetUsername(OptionalNumberArg(L, 1)));
    return 1;
}

/**
 * Get the current user id.
 * @name get_user_id
 */
int Wavedash_GetUserId(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetUserId());
    return 1;
}

/**
 * Request a user JWT.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'getUserJwt' or as a return value if the function is called from
 * a coroutine.
 * @name get_user_jwt_async
 */
int Wavedash_GetUserJwtAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_GetUserJwtAsync();
    }
    return AwaitAsyncEvent(L, "getUserJwt");
}

/**
 * Get launch parameters.
 * @name get_launch_params
 */
int Wavedash_GetLaunchParams(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_GetLaunchParams());
    return 1;
}

/**
 * List friends.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'listFriends' or as a return value if the function is called from
 * a coroutine.
 * @name list_friends_async
 */
int Wavedash_ListFriendsAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ListFriendsAsync();
    }
    return AwaitAsyncEvent(L, "listFriends");
}

/**
 * Get a user avatar URL.
 * @name get_user_avatar_url
 * @number user_id
 * @number size?
 */
int Wavedash_GetUserAvatarUrl(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeString(L, WavedashJs_GetUserAvatarUrl(luaL_checknumber(L, 1), OptionalNumberArg(L, 2)));
    return 1;
}

/**
 * Get a leaderboard.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'getLeaderboard' or as a return value if the function is called from
 * a coroutine.
 * @name get_leaderboard_async
 * @string name
 */
int Wavedash_GetLeaderboardAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_GetLeaderboardAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "getLeaderboard");
}

/**
 * Get or create a leaderboard.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'getOrCreateLeaderboard' or as a return value if the function is called from
 * a coroutine.
 * @name get_or_create_leaderboard_async
 * @string name
 * @number sort_order
 * @number display_type
 */
int Wavedash_GetOrCreateLeaderboardAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_GetOrCreateLeaderboardAsync(luaL_checkstring(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    }
    return AwaitAsyncEvent(L, "getOrCreateLeaderboard");
}

/**
 * Get leaderboard entry count.
 * @name get_leaderboard_entry_count
 * @number leaderboard_id
 */
int Wavedash_GetLeaderboardEntryCount(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetLeaderboardEntryCount(luaL_checknumber(L, 1)));
    return 1;
}

/**
 * Get the current user's leaderboard entries.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'getMyLeaderboardEntries' or as a return value if the function is called from
 * a coroutine.
 * @name get_my_leaderboard_entries_async
 * @number leaderboard_id
 */
int Wavedash_GetMyLeaderboardEntriesAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_GetMyLeaderboardEntriesAsync(luaL_checknumber(L, 1));
    }
    return AwaitAsyncEvent(L, "getMyLeaderboardEntries");
}

/**
 * List leaderboard entries around a user.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'listLeaderboardEntriesAroundUser' or as a return value if the function is called from
 * a coroutine.
 * @name list_leaderboard_entries_around_user_async
 * @number leaderboard_id
 * @number count_ahead
 * @number count_behind
 * @boolean friends_only?
 */
int Wavedash_ListLeaderboardEntriesAroundUserAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ListLeaderboardEntriesAroundUserAsync(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), OptionalBoolArg(L, 4));
    }
    return AwaitAsyncEvent(L, "listLeaderboardEntriesAroundUser");
}

/**
 * List leaderboard entries.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'listLeaderboardEntries' or as a return value if the function is called from
 * a coroutine.
 * @name list_leaderboard_entries_async
 * @number leaderboard_id
 * @number offset
 * @number limit
 * @boolean friends_only?
 */
int Wavedash_ListLeaderboardEntriesAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ListLeaderboardEntriesAsync(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), OptionalBoolArg(L, 4));
    }
    return AwaitAsyncEvent(L, "listLeaderboardEntries");
}

/**
 * Upload a leaderboard score.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'uploadLeaderboardScore' or as a return value if the function is called from
 * a coroutine.
 * @name upload_leaderboard_score_async
 * @number leaderboard_id
 * @number score
 * @boolean keep_best
 * @number ugc_id?
 */
int Wavedash_UploadLeaderboardScoreAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_UploadLeaderboardScoreAsync(luaL_checknumber(L, 1), luaL_checknumber(L, 2), lua_toboolean(L, 3) ? 1 : 0, OptionalNumberArg(L, 4));
    }
    return AwaitAsyncEvent(L, "uploadLeaderboardScore");
}

/**
 * Create a UGC item.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'createUGCItem' or as a return value if the function is called from
 * a coroutine.
 * @name create_ugc_item_async
 * @number ugc_type
 * @string title?
 * @string description?
 * @number visibility?
 * @string file_path?
 */
int Wavedash_CreateUGCItemAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_CreateUGCItemAsync(luaL_checknumber(L, 1), OptionalStringArg(L, 2), OptionalStringArg(L, 3), OptionalNumberArg(L, 4), OptionalStringArg(L, 5));
    }
    return AwaitAsyncEvent(L, "createUGCItem");
}

/**
 * Update a UGC item.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'updateUGCItem' or as a return value if the function is called from
 * a coroutine.
 * @name update_ugc_item_async
 * @number ugc_id
 * @string title?
 * @string description?
 * @number visibility?
 * @string file_path?
 */
int Wavedash_UpdateUGCItemAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_UpdateUGCItemAsync(luaL_checknumber(L, 1), OptionalStringArg(L, 2), OptionalStringArg(L, 3), OptionalNumberArg(L, 4), OptionalStringArg(L, 5));
    }
    return AwaitAsyncEvent(L, "updateUGCItem");
}

/**
 * Download a UGC item.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'downloadUGCItem' or as a return value if the function is called from
 * a coroutine.
 * @name download_ugc_item_async
 * @number ugc_id
 * @string file_path
 */
int Wavedash_DownloadUGCItemAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_DownloadUGCItemAsync(luaL_checknumber(L, 1), luaL_checkstring(L, 2));
    }
    return AwaitAsyncEvent(L, "downloadUGCItem");
}

/**
 * Delete a remote file.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'deleteRemoteFile' or as a return value if the function is called from
 * a coroutine.
 * @name delete_remote_file_async
 * @string file_path
 */
int Wavedash_DeleteRemoteFileAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_DeleteRemoteFileAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "deleteRemoteFile");
}

/**
 * Download a remote file.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'downloadRemoteFile' or as a return value if the function is called from
 * a coroutine.
 * @name download_remote_file_async
 * @string file_path
 */
int Wavedash_DownloadRemoteFileAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_DownloadRemoteFileAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "downloadRemoteFile");
}

/**
 * Upload a remote file.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'uploadRemoteFile' or as a return value if the function is called from
 * a coroutine.
 * @name upload_remote_file_async
 * @string file_path
 */
int Wavedash_UploadRemoteFileAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_UploadRemoteFileAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "uploadRemoteFile");
}

/**
 * List a remote directory.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'listRemoteDirectory' or as a return value if the function is called from
 * a coroutine.
 * @name list_remote_directory_async
 * @string path
 */
int Wavedash_ListRemoteDirectoryAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ListRemoteDirectoryAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "listRemoteDirectory");
}

/**
 * Download a remote directory.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'downloadRemoteDirectory' or as a return value if the function is called from
 * a coroutine.
 * @name download_remote_directory_async
 * @string path
 */
int Wavedash_DownloadRemoteDirectoryAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_DownloadRemoteDirectoryAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "downloadRemoteDirectory");
}

/**
 * Write a local file.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'writeLocalFile' or as a return value if the function is called from
 * a coroutine.
 * @name write_local_file_async
 * @string file_path
 * @string data
 */
int Wavedash_WriteLocalFileAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);

        size_t data_length = 0;
        const char* data = luaL_checklstring(L, 2, &data_length);
        WavedashJs_WriteLocalFileAsync(luaL_checkstring(L, 1), data, (uint32_t) data_length);
    }
    return AwaitAsyncEvent(L, "writeLocalFile");
}

/**
 * Read a local file.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'readLocalFile' or as a return value if the function is called from
 * a coroutine.
 * @name read_local_file_async
 * @string file_path
 */
int Wavedash_ReadLocalFileAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ReadLocalFileAsync(luaL_checkstring(L, 1));
    }
    return AwaitAsyncEvent(L, "readLocalFile");
}

/**
 * Get an achievement.
 * @name get_achievement
 * @string identifier
 */
int Wavedash_GetAchievement(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_GetAchievement(luaL_checkstring(L, 1)));
    return 1;
}

/**
 * Get a stat.
 * @name get_stat
 * @string identifier
 */
int Wavedash_GetStat(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetStat(luaL_checkstring(L, 1)));
    return 1;
}

/**
 * Set an achievement.
 * @name set_achievement
 * @string identifier
 * @boolean store_now?
 */
int Wavedash_SetAchievement(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_SetAchievement(luaL_checkstring(L, 1), OptionalBoolArg(L, 2)));
    return 1;
}

/**
 * Set a stat.
 * @name set_stat
 * @string identifier
 * @number value
 * @boolean store_now?
 */
int Wavedash_SetStat(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_SetStat(luaL_checkstring(L, 1), luaL_checknumber(L, 2), OptionalBoolArg(L, 3)));
    return 1;
}

/**
 * Request stats.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'requestStats' or as a return value if the function is called from
 * a coroutine.
 * @name request_stats_async
 */
int Wavedash_RequestStatsAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_RequestStatsAsync();
    }
    return AwaitAsyncEvent(L, "requestStats");
}

/**
 * Store stats.
 * @name store_stats
 */
int Wavedash_StoreStats(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_StoreStats());
    return 1;
}

/**
 * Get the maximum P2P payload size.
 * @name get_p2p_max_payload_size
 */
int Wavedash_GetP2PMaxPayloadSize(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetP2PMaxPayloadSize());
    return 1;
}

/**
 * Get the maximum number of incoming P2P messages.
 * @name get_p2p_max_incoming_messages
 */
int Wavedash_GetP2PMaxIncomingMessages(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetP2PMaxIncomingMessages());
    return 1;
}

/**
 * Get the P2P outgoing message buffer.
 * @name get_p2p_outgoing_message_buffer
 */
int Wavedash_GetP2POutgoingMessageBuffer(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    uint32_t length = 0;
    PushAndFreeBytes(L, WavedashJs_GetP2POutgoingMessageBuffer(&length), length);
    return 1;
}

/**
 * Send a P2P message.
 * @name send_p2p_message
 * @number to_user_id?
 * @number app_channel?
 * @boolean reliable?
 * @string payload
 * @number payload_size?
 */
int Wavedash_SendP2PMessage(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    size_t payload_length = 0;
    const char* payload = luaL_checklstring(L, 4, &payload_length);
    lua_pushboolean(L, WavedashJs_SendP2PMessage(OptionalNumberArg(L, 1), OptionalNumberArg(L, 2), OptionalBoolArg(L, 3), payload, (uint32_t) payload_length, OptionalNumberArg(L, 5)));
    return 1;
}

/**
 * Broadcast a P2P message.
 * @name broadcast_p2p_message
 * @number app_channel?
 * @boolean reliable?
 * @string payload
 * @number payload_size?
 */
int Wavedash_BroadcastP2PMessage(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    size_t payload_length = 0;
    const char* payload = luaL_checklstring(L, 3, &payload_length);
    lua_pushboolean(L, WavedashJs_BroadcastP2PMessage(OptionalNumberArg(L, 1), OptionalBoolArg(L, 2), payload, (uint32_t) payload_length, OptionalNumberArg(L, 4)));
    return 1;
}

/**
 * Read a P2P message from a channel.
 * @name read_p2p_message_from_channel
 * @number app_channel
 */
int Wavedash_ReadP2PMessageFromChannel(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_ReadP2PMessageFromChannel(luaL_checknumber(L, 1)));
    return 1;
}

/**
 * Drain a P2P channel to a buffer.
 * @name drain_p2p_channel_to_buffer
 * @number app_channel
 */
int Wavedash_DrainP2PChannelToBuffer(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    uint32_t length = 0;
    PushAndFreeBytes(L, WavedashJs_DrainP2PChannelToBuffer(luaL_checknumber(L, 1), &length), length);
    return 1;
}

/**
 * Create a lobby.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'createLobby' or as a return value if the function is called from
 * a coroutine.
 * @name create_lobby_async
 * @number visibility
 * @number max_players
 */
int Wavedash_CreateLobbyAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_CreateLobbyAsync(luaL_checknumber(L, 1), OptionalNumberArg(L, 2));
    }
    return AwaitAsyncEvent(L, "createLobby");
}

/**
 * Join a lobby.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'joinLobby' or as a return value if the function is called from
 * a coroutine.
 * @name join_lobby_async
 * @number lobby_id
 */
int Wavedash_JoinLobbyAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_JoinLobbyAsync(luaL_checknumber(L, 1));
    }
    return AwaitAsyncEvent(L, "joinLobby");
}

/**
 * List available lobbies.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'listAvailableLobbies' or as a return value if the function is called from
 * a coroutine.
 * @name list_available_lobbies_async
 * @boolean friends_only?
 */
int Wavedash_ListAvailableLobbiesAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_ListAvailableLobbiesAsync(OptionalBoolArg(L, 1));
    }
    return AwaitAsyncEvent(L, "listAvailableLobbies");
}

/**
 * Get lobby users.
 * @name get_lobby_users
 * @number lobby_id
 */
int Wavedash_GetLobbyUsers(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_GetLobbyUsers(luaL_checknumber(L, 1)));
    return 1;
}

/**
 * Get the number of lobby users.
 * @name get_num_lobby_users
 * @number lobby_id
 */
int Wavedash_GetNumLobbyUsers(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushnumber(L, WavedashJs_GetNumLobbyUsers(luaL_checknumber(L, 1)));
    return 1;
}

/**
 * Get the lobby host id.
 * @name get_lobby_host_id
 * @number lobby_id
 */
int Wavedash_GetLobbyHostId(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_GetLobbyHostId(luaL_checknumber(L, 1)));
    return 1;
}

/**
 * Get lobby data.
 * @name get_lobby_data
 * @number lobby_id
 * @string key
 */
int Wavedash_GetLobbyData(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    PushAndFreeJson(L, WavedashJs_GetLobbyData(luaL_checknumber(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

/**
 * Set lobby data.
 * @name set_lobby_data
 * @number lobby_id
 * @string key
 * @any value
 */
int Wavedash_SetLobbyData(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    std::string value_json;
    if (!LuaValueToJsonLiteral(L, 3, value_json))
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, WavedashJs_SetLobbyData(luaL_checknumber(L, 1), luaL_checkstring(L, 2), value_json.c_str()));
    return 1;
}

/**
 * Delete lobby data.
 * @name delete_lobby_data
 * @number lobby_id
 * @string key
 */
int Wavedash_DeleteLobbyData(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_DeleteLobbyData(luaL_checknumber(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

/**
 * Leave a lobby.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'leaveLobby' or as a return value if the function is called from
 * a coroutine.
 * @name leave_lobby_async
 * @number lobby_id
 */
int Wavedash_LeaveLobbyAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_LeaveLobbyAsync(luaL_checknumber(L, 1));
    }
    return AwaitAsyncEvent(L, "leaveLobby");
}

/**
 * Send a lobby message.
 * @name send_lobby_message
 * @number lobby_id
 * @string message
 */
int Wavedash_SendLobbyMessage(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, WavedashJs_SendLobbyMessage(luaL_checknumber(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

/**
 * Invite a user to a lobby.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'inviteUserToLobby' or as a return value if the function is called from
 * a coroutine.
 * @name invite_user_to_lobby_async
 * @number lobby_id
 * @number user_id
 */
int Wavedash_InviteUserToLobbyAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_InviteUserToLobbyAsync(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    }
    return AwaitAsyncEvent(L, "inviteUserToLobby");
}

/**
 * Get a lobby invite link.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'getLobbyInviteLink' or as a return value if the function is called from
 * a coroutine.
 * @name get_lobby_invite_link_async
 * @boolean copy_to_clipboard?
 */
int Wavedash_GetLobbyInviteLinkAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_GetLobbyInviteLinkAsync(OptionalBoolArg(L, 1));
    }
    return AwaitAsyncEvent(L, "getLobbyInviteLink");
}

/**
 * Update user presence.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'updateUserPresence' or as a return value if the function is called from
 * a coroutine.
 * @name update_user_presence_async
 * @string data_json?
 */
int Wavedash_UpdateUserPresenceAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_UpdateUserPresenceAsync(RawJsonStringArg(L, 1));
    }
    return AwaitAsyncEvent(L, "updateUserPresence");
}

/**
 * Ensure a gameplay JWT is available.
 * This is an asynchronous function. The result will be delivered as an event
 * with id 'ensureGameplayJwt' or as a return value if the function is called from
 * a coroutine.
 * @name ensure_gameplay_jwt_async
 */
int Wavedash_EnsureGameplayJwtAsync(lua_State* L)
{
    {
        DM_LUA_STACK_CHECK(L, 0);
        WavedashJs_EnsureGameplayJwtAsync();
    }
    return AwaitAsyncEvent(L, "ensureGameplayJwt");
}

static const luaL_reg Module_methods[] =
{
    {"init", Wavedash_Init},
    {"ready_for_events", Wavedash_ReadyForEvents},
    {"update_load_progress_zero_to_one", Wavedash_UpdateLoadProgressZeroToOne},
    {"load_complete", Wavedash_LoadComplete},
    {"toggle_overlay", Wavedash_ToggleOverlay},
    {"is_fullscreen", Wavedash_IsFullscreen},
    {"request_fullscreen_async", Wavedash_RequestFullscreenAsync},
    {"toggle_fullscreen_async", Wavedash_ToggleFullscreenAsync},
    {"get_user", Wavedash_GetUser},
    {"get_username", Wavedash_GetUsername},
    {"get_user_id", Wavedash_GetUserId},
    {"get_user_jwt_async", Wavedash_GetUserJwtAsync},
    {"get_launch_params", Wavedash_GetLaunchParams},
    {"list_friends_async", Wavedash_ListFriendsAsync},
    {"get_user_avatar_url", Wavedash_GetUserAvatarUrl},
    {"get_leaderboard_async", Wavedash_GetLeaderboardAsync},
    {"get_or_create_leaderboard_async", Wavedash_GetOrCreateLeaderboardAsync},
    {"get_leaderboard_entry_count", Wavedash_GetLeaderboardEntryCount},
    {"get_my_leaderboard_entries_async", Wavedash_GetMyLeaderboardEntriesAsync},
    {"list_leaderboard_entries_around_user_async", Wavedash_ListLeaderboardEntriesAroundUserAsync},
    {"list_leaderboard_entries_async", Wavedash_ListLeaderboardEntriesAsync},
    {"upload_leaderboard_score_async", Wavedash_UploadLeaderboardScoreAsync},
    {"create_ugc_item_async", Wavedash_CreateUGCItemAsync},
    {"update_ugc_item_async", Wavedash_UpdateUGCItemAsync},
    {"download_ugc_item_async", Wavedash_DownloadUGCItemAsync},
    {"delete_remote_file_async", Wavedash_DeleteRemoteFileAsync},
    {"download_remote_file_async", Wavedash_DownloadRemoteFileAsync},
    {"upload_remote_file_async", Wavedash_UploadRemoteFileAsync},
    {"list_remote_directory_async", Wavedash_ListRemoteDirectoryAsync},
    {"download_remote_directory_async", Wavedash_DownloadRemoteDirectoryAsync},
    {"write_local_file_async", Wavedash_WriteLocalFileAsync},
    {"read_local_file_async", Wavedash_ReadLocalFileAsync},
    {"get_achievement", Wavedash_GetAchievement},
    {"get_stat", Wavedash_GetStat},
    {"set_achievement", Wavedash_SetAchievement},
    {"set_stat", Wavedash_SetStat},
    {"request_stats_async", Wavedash_RequestStatsAsync},
    {"store_stats", Wavedash_StoreStats},
    {"get_p2p_max_payload_size", Wavedash_GetP2PMaxPayloadSize},
    {"get_p2p_max_incoming_messages", Wavedash_GetP2PMaxIncomingMessages},
    {"get_p2p_outgoing_message_buffer", Wavedash_GetP2POutgoingMessageBuffer},
    {"send_p2p_message", Wavedash_SendP2PMessage},
    {"broadcast_p2p_message", Wavedash_BroadcastP2PMessage},
    {"read_p2p_message_from_channel", Wavedash_ReadP2PMessageFromChannel},
    {"drain_p2p_channel_to_buffer", Wavedash_DrainP2PChannelToBuffer},
    {"create_lobby_async", Wavedash_CreateLobbyAsync},
    {"join_lobby_async", Wavedash_JoinLobbyAsync},
    {"list_available_lobbies_async", Wavedash_ListAvailableLobbiesAsync},
    {"get_lobby_users", Wavedash_GetLobbyUsers},
    {"get_num_lobby_users", Wavedash_GetNumLobbyUsers},
    {"get_lobby_host_id", Wavedash_GetLobbyHostId},
    {"get_lobby_data", Wavedash_GetLobbyData},
    {"set_lobby_data", Wavedash_SetLobbyData},
    {"delete_lobby_data", Wavedash_DeleteLobbyData},
    {"leave_lobby_async", Wavedash_LeaveLobbyAsync},
    {"send_lobby_message", Wavedash_SendLobbyMessage},
    {"invite_user_to_lobby_async", Wavedash_InviteUserToLobbyAsync},
    {"get_lobby_invite_link_async", Wavedash_GetLobbyInviteLinkAsync},
    {"update_user_presence_async", Wavedash_UpdateUserPresenceAsync},
    {"ensure_gameplay_jwt_async", Wavedash_EnsureGameplayJwtAsync},
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

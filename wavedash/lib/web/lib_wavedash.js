// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

var LibWavedash = {

    $WavedashJs: {
        eventCallback: null,

        invokeCallback: function(event, payload) {
            console.log("Payload", event, payload);
            if (payload != null) {
                var event_c = stringToNewUTF8(event);
                var payload_json = JSON.stringify(payload);
                var payload_json_c = stringToNewUTF8(payload_json);
                var payload_json_len = lengthBytesUTF8(payload_json);
                {{{ makeDynCall("viii", "WavedashJs.eventCallback")}}}(event_c, payload_json_c, payload_json_len);
                Module._free(payload_json_c);
                Module._free(event_c);
            }
            else {
                var event_c = stringToNewUTF8(event);
                var payload_json_c = 0;
                var payload_json_len = 0;
                {{{ makeDynCall("viii", "WavedashJs.eventCallback")}}}(event_c, payload_json_c, payload_json_len);
                Module._free(event_c);
            }
        },

        initEvents: function() {
            for (var key in window.Wavedash.Events) {
                if (!window.Wavedash.Events.hasOwnProperty(key)) {
                    continue;
                }

                const event = window.Wavedash.Events[key];
                window.Wavedash.on(event, function(payload) {
                    // console.log("on", event, payload);
                    WavedashJs.invokeCallback(event, payload);
                });
            }
        },

        call: function(method, args) {
            return window.Wavedash[method].apply(window.Wavedash, args || []);
        },

        callPromise: function(method, args) {
            var p = Promise.resolve(WavedashJs.call(method, args));
            p.then(
                function(response) {
                    WavedashJs.invokeCallback(method, response.data);
                },
                function(err) {
                    console.log(err);
                    WavedashJs.invokeCallback(method, null);
                }
            );
        },

        optionalNumber: function(value) {
            return Number.isNaN(value) ? undefined : value;
        },

        optionalBool: function(value) {
            return value < 0 ? undefined : !!value;
        },

        optionalString: function(ptr) {
            return ptr ? UTF8ToString(ptr) : undefined;
        },

        optionalJson: function(ptr) {
            return ptr ? JSON.parse(UTF8ToString(ptr)) : undefined;
        },

        heapBytes: function(ptr, len) {
            if (!ptr || !len) {
                return new Uint8Array(0);
            }
            return HEAPU8.slice(ptr, ptr + len);
        },

        allocString: function(value) {
            if (value === null || value === undefined) {
                return 0;
            }
            return stringToNewUTF8(String(value));
        },

        allocJson: function(value) {
            if (value === null || value === undefined) {
                return 0;
            }
            return stringToNewUTF8(JSON.stringify(value));
        },

        allocBytes: function(value, outLen) {
            var bytes = value || new Uint8Array(0);
            if (outLen) {
                HEAP32[outLen >> 2] = bytes.length;
            }
            if (!bytes.length) {
                return 0;
            }
            var ptr = Module._malloc(bytes.length);
            HEAPU8.set(bytes, ptr);
            return ptr;
        }
    },

    WavedashJs_Free: function(ptr) {
        if (ptr) {
            Module._free(ptr);
        }
    },

    WavedashJs_Init: function(eventCallback) {
        WavedashJs.eventCallback = eventCallback;
        var result = WavedashJs.call("init", []);
        WavedashJs.initEvents();
        return result ? 1 : 0;
    },

    WavedashJs_ReadyForEvents: function() {
        WavedashJs.call("readyForEvents", []);
    },

    WavedashJs_UpdateLoadProgressZeroToOne: function(progress) {
        WavedashJs.call("updateLoadProgressZeroToOne", [progress]);
    },

    WavedashJs_LoadComplete: function() {
        WavedashJs.call("loadComplete", []);
    },

    WavedashJs_ToggleOverlay: function() {
        WavedashJs.call("toggleOverlay", []);
    },

    WavedashJs_IsFullscreen: function() {
        return WavedashJs.call("isFullscreen", []) ? 1 : 0;
    },

    WavedashJs_RequestFullscreenAsync: function(fullscreen) {
        WavedashJs.callPromise("requestFullscreen", [!!fullscreen]);
    },

    WavedashJs_ToggleFullscreenAsync: function() {
        WavedashJs.callPromise("toggleFullscreen", []);
    },

    WavedashJs_GetUser: function() {
        return WavedashJs.allocJson(WavedashJs.call("getUser", []));
    },

    WavedashJs_GetUsername: function(userId) {
        var normalizedUserId = WavedashJs.optionalNumber(userId);
        var args = normalizedUserId === undefined ? [] : [normalizedUserId];
        return WavedashJs.allocString(WavedashJs.call("getUsername", args));
    },

    WavedashJs_GetUserId: function() {
        return WavedashJs.call("getUserId", []);
    },

    WavedashJs_GetUserJwtAsync: function() {
        WavedashJs.callPromise("getUserJwt", []);
    },

    WavedashJs_GetLaunchParams: function() {
        return WavedashJs.allocJson(WavedashJs.call("getLaunchParams", []));
    },

    WavedashJs_ListFriendsAsync: function() {
        WavedashJs.callPromise("listFriends", []);
    },

    WavedashJs_GetUserAvatarUrl: function(userId, size) {
        return WavedashJs.allocString(WavedashJs.call("getUserAvatarUrl", [userId, WavedashJs.optionalNumber(size)]));
    },

    WavedashJs_GetLeaderboardAsync: function(name) {
        WavedashJs.callPromise("getLeaderboard", [UTF8ToString(name)]);
    },

    WavedashJs_GetOrCreateLeaderboardAsync: function(name, sortOrder, displayType) {
        WavedashJs.callPromise("getOrCreateLeaderboard", [UTF8ToString(name), sortOrder, displayType]);
    },

    WavedashJs_GetLeaderboardEntryCount: function(leaderboardId) {
        return WavedashJs.call("getLeaderboardEntryCount", [leaderboardId]);
    },

    WavedashJs_GetMyLeaderboardEntriesAsync: function(leaderboardId) {
        WavedashJs.callPromise("getMyLeaderboardEntries", [leaderboardId]);
    },

    WavedashJs_ListLeaderboardEntriesAroundUserAsync: function(leaderboardId, countAhead, countBehind, friendsOnly) {
        WavedashJs.callPromise("listLeaderboardEntriesAroundUser", [leaderboardId, countAhead, countBehind, WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_ListLeaderboardEntriesAsync: function(leaderboardId, offset, limit, friendsOnly) {
        WavedashJs.callPromise("listLeaderboardEntries", [leaderboardId, offset, limit, WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_UploadLeaderboardScoreAsync: function(leaderboardId, score, keepBest, ugcId) {
        WavedashJs.callPromise("uploadLeaderboardScore", [leaderboardId, score, !!keepBest, WavedashJs.optionalNumber(ugcId)]);
    },

    WavedashJs_CreateUGCItemAsync: function(ugcType, title, description, visibility, filePath) {
        WavedashJs.callPromise("createUGCItem", [
            ugcType,
            WavedashJs.optionalString(title),
            WavedashJs.optionalString(description),
            WavedashJs.optionalNumber(visibility),
            WavedashJs.optionalString(filePath)
        ]);
    },

    WavedashJs_UpdateUGCItemAsync: function(ugcId, title, description, visibility, filePath) {
        WavedashJs.callPromise("updateUGCItem", [
            ugcId,
            WavedashJs.optionalString(title),
            WavedashJs.optionalString(description),
            WavedashJs.optionalNumber(visibility),
            WavedashJs.optionalString(filePath)
        ]);
    },

    WavedashJs_DownloadUGCItemAsync: function(ugcId, filePath) {
        WavedashJs.callPromise("downloadUGCItem", [ugcId, UTF8ToString(filePath)]);
    },

    WavedashJs_DeleteRemoteFileAsync: function(filePath) {
        WavedashJs.callPromise("deleteRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_DownloadRemoteFileAsync: function(filePath) {
        WavedashJs.callPromise("downloadRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_UploadRemoteFileAsync: function(filePath) {
        WavedashJs.callPromise("uploadRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_ListRemoteDirectoryAsync: function(path) {
        WavedashJs.callPromise("listRemoteDirectory", [UTF8ToString(path)]);
    },

    WavedashJs_DownloadRemoteDirectoryAsync: function(path) {
        WavedashJs.callPromise("downloadRemoteDirectory", [UTF8ToString(path)]);
    },

    WavedashJs_WriteLocalFileAsync: function(filePath, dataPtr, dataLen) {
        WavedashJs.callPromise("writeLocalFile", [UTF8ToString(filePath), WavedashJs.heapBytes(dataPtr, dataLen)]);
    },

    WavedashJs_ReadLocalFileAsync: function(filePath) {
        WavedashJs.callPromise("readLocalFile", [UTF8ToString(filePath)]);
        return 0;
    },

    WavedashJs_GetAchievement: function(identifier) {
        return WavedashJs.call("getAchievement", [UTF8ToString(identifier)]) ? 1 : 0;
    },

    WavedashJs_GetStat: function(identifier) {
        return WavedashJs.call("getStat", [UTF8ToString(identifier)]);
    },

    WavedashJs_SetAchievement: function(identifier, storeNow) {
        return WavedashJs.call("setAchievement", [UTF8ToString(identifier), WavedashJs.optionalBool(storeNow)]) ? 1 : 0;
    },

    WavedashJs_SetStat: function(identifier, value, storeNow) {
        return WavedashJs.call("setStat", [UTF8ToString(identifier), value, WavedashJs.optionalBool(storeNow)]) ? 1 : 0;
    },

    WavedashJs_RequestStatsAsync: function() {
        WavedashJs.callPromise("requestStats", []);
    },

    WavedashJs_StoreStats: function() {
        return WavedashJs.call("storeStats", []) ? 1 : 0;
    },

    WavedashJs_GetP2PMaxPayloadSize: function() {
        return WavedashJs.call("getP2PMaxPayloadSize", []);
    },

    WavedashJs_GetP2PMaxIncomingMessages: function() {
        return WavedashJs.call("getP2PMaxIncomingMessages", []);
    },

    WavedashJs_GetP2POutgoingMessageBuffer: function(outLen) {
        return WavedashJs.allocBytes(WavedashJs.call("getP2POutgoingMessageBuffer", []), outLen);
    },

    WavedashJs_SendP2PMessage: function(toUserId, appChannel, reliable, payloadPtr, payloadLen, payloadSize) {
        return WavedashJs.call("sendP2PMessage", [
            WavedashJs.optionalNumber(toUserId),
            WavedashJs.optionalNumber(appChannel),
            WavedashJs.optionalBool(reliable),
            WavedashJs.heapBytes(payloadPtr, payloadLen),
            WavedashJs.optionalNumber(payloadSize)
        ]) ? 1 : 0;
    },

    WavedashJs_BroadcastP2PMessage: function(appChannel, reliable, payloadPtr, payloadLen, payloadSize) {
        return WavedashJs.call("broadcastP2PMessage", [
            WavedashJs.optionalNumber(appChannel),
            WavedashJs.optionalBool(reliable),
            WavedashJs.heapBytes(payloadPtr, payloadLen),
            WavedashJs.optionalNumber(payloadSize)
        ]) ? 1 : 0;
    },

    WavedashJs_ReadP2PMessageFromChannel: function(appChannel) {
        return WavedashJs.allocJson(WavedashJs.call("readP2PMessageFromChannel", [appChannel]));
    },

    WavedashJs_DrainP2PChannelToBuffer: function(appChannel, outLen) {
        return WavedashJs.allocBytes(WavedashJs.call("drainP2PChannelToBuffer", [appChannel]), outLen);
    },

    WavedashJs_CreateLobbyAsync: function(visibility, maxPlayers) {
        WavedashJs.callPromise("createLobby", [visibility, WavedashJs.optionalNumber(maxPlayers)]);
    },

    WavedashJs_JoinLobbyAsync: function(lobbyId) {
        WavedashJs.callPromise("joinLobby", [lobbyId]);
    },

    WavedashJs_ListAvailableLobbiesAsync: function(friendsOnly) {
        WavedashJs.callPromise("listAvailableLobbies", [WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_GetLobbyUsers: function(lobbyId) {
        return WavedashJs.allocJson(WavedashJs.call("getLobbyUsers", [lobbyId]));
    },

    WavedashJs_GetNumLobbyUsers: function(lobbyId) {
        return WavedashJs.call("getNumLobbyUsers", [lobbyId]);
    },

    WavedashJs_GetLobbyHostId: function(lobbyId) {
        return WavedashJs.allocJson(WavedashJs.call("getLobbyHostId", [lobbyId]));
    },

    WavedashJs_GetLobbyData: function(lobbyId, key) {
        return WavedashJs.allocJson(WavedashJs.call("getLobbyData", [lobbyId, UTF8ToString(key)]));
    },

    WavedashJs_SetLobbyData: function(lobbyId, key, valueJson) {
        return WavedashJs.call("setLobbyData", [lobbyId, UTF8ToString(key), WavedashJs.optionalJson(valueJson)]) ? 1 : 0;
    },

    WavedashJs_DeleteLobbyData: function(lobbyId, key) {
        return WavedashJs.call("deleteLobbyData", [lobbyId, UTF8ToString(key)]) ? 1 : 0;
    },

    WavedashJs_LeaveLobbyAsync: function(lobbyId) {
        WavedashJs.callPromise("leaveLobby", [lobbyId]);
    },

    WavedashJs_SendLobbyMessage: function(lobbyId, message) {
        return WavedashJs.call("sendLobbyMessage", [lobbyId, UTF8ToString(message)]) ? 1 : 0;
    },

    WavedashJs_InviteUserToLobbyAsync: function(lobbyId, userId) {
        WavedashJs.callPromise("inviteUserToLobby", [lobbyId, userId]);
    },

    WavedashJs_GetLobbyInviteLinkAsync: function(copyToClipboard) {
        WavedashJs.callPromise("getLobbyInviteLink", [WavedashJs.optionalBool(copyToClipboard)]);
    },

    WavedashJs_UpdateUserPresenceAsync: function(dataJson) {
        WavedashJs.callPromise("updateUserPresence", [WavedashJs.optionalJson(dataJson)]);
    },

    WavedashJs_EnsureGameplayJwtAsync: function() {
        WavedashJs.callPromise("ensureGameplayJwt", []);
    }
}

autoAddDeps(LibWavedash, '$WavedashJs');
addToLibrary(LibWavedash);

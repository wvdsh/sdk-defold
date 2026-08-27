// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

var LibWavedash = {

    $WavedashJs: {
        eventCallback: null,
        nextAsyncRequestId: 0,

        invokeCallback: function(event, payload, requestId) {
            // console.log("Payload", event, payload);
            if (payload != null) {
                var event_c = stringToNewUTF8(event);
                var payload_json = JSON.stringify(payload);
                var payload_json_c = stringToNewUTF8(payload_json);
                var payload_json_len = lengthBytesUTF8(payload_json);
                {{{ makeDynCall("viiii", "WavedashJs.eventCallback")}}}(event_c, requestId || 0, payload_json_c, payload_json_len);
                Module._free(payload_json_c);
                Module._free(event_c);
            }
            else {
                var event_c = stringToNewUTF8(event);
                var payload_json_c = 0;
                var payload_json_len = 0;
                {{{ makeDynCall("viiii", "WavedashJs.eventCallback")}}}(event_c, requestId || 0, payload_json_c, payload_json_len);
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
            console.assert(window.Wavedash[method], "Wavedash SDK does not have any method named '%s'", method);
            return window.Wavedash[method].apply(window.Wavedash, args || []);
        },

        callPromise: function(method, args) {
            WavedashJs.nextAsyncRequestId = (WavedashJs.nextAsyncRequestId + 1) >>> 0;
            if (!WavedashJs.nextAsyncRequestId) {
                WavedashJs.nextAsyncRequestId = 1;
            }
            var requestId = WavedashJs.nextAsyncRequestId;
            var p = Promise.resolve(WavedashJs.call(method, args));
            p.then(
                function(response) {
                    WavedashJs.invokeCallback(method, response, requestId);
                },
                function(err) {
                    WavedashJs.invokeCallback(method, { success: false, data: null, message: String(err) }, requestId);
                }
            );
            return requestId;
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

    WavedashJs_Init: function(config_json, eventCallback) {
        WavedashJs.eventCallback = eventCallback;
        var result = WavedashJs.call("init", [WavedashJs.optionalJson(config_json)]);
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
        return WavedashJs.callPromise("requestFullscreen", [!!fullscreen]);
    },

    WavedashJs_ToggleFullscreenAsync: function() {
        return WavedashJs.callPromise("toggleFullscreen", []);
    },

    WavedashJs_GetUser: function() {
        return WavedashJs.allocJson(WavedashJs.call("getUser", []));
    },

    WavedashJs_GetUsername: function(userId) {
        var normalizedUserId = WavedashJs.optionalString(userId);
        var args = normalizedUserId === undefined ? [] : [normalizedUserId];
        return WavedashJs.allocString(WavedashJs.call("getUsername", args));
    },

    WavedashJs_GetUserId: function() {
        return WavedashJs.allocString(WavedashJs.call("getUserId", []));
    },

    WavedashJs_GetUserJwtAsync: function() {
        return WavedashJs.callPromise("getUserJwt", []);
    },

    WavedashJs_GetLaunchParams: function() {
        return WavedashJs.allocJson(WavedashJs.call("getLaunchParams", []));
    },

    WavedashJs_ListFriendsAsync: function() {
        return WavedashJs.callPromise("listFriends", []);
    },

    WavedashJs_GetUserAvatarUrl: function(userId, size) {
        return WavedashJs.allocString(WavedashJs.call("getUserAvatarUrl", [UTF8ToString(userId), WavedashJs.optionalNumber(size)]));
    },

    WavedashJs_GetLeaderboardAsync: function(name) {
        return WavedashJs.callPromise("getLeaderboard", [UTF8ToString(name)]);
    },

    WavedashJs_GetOrCreateLeaderboardAsync: function(name, sortOrder, displayType) {
        return WavedashJs.callPromise("getOrCreateLeaderboard", [UTF8ToString(name), sortOrder, displayType]);
    },

    WavedashJs_GetLeaderboardEntryCount: function(leaderboardId) {
        return WavedashJs.call("getLeaderboardEntryCount", [UTF8ToString(leaderboardId)]);
    },

    WavedashJs_GetMyLeaderboardEntriesAsync: function(leaderboardId) {
        return WavedashJs.callPromise("getMyLeaderboardEntries", [UTF8ToString(leaderboardId)]);
    },

    WavedashJs_ListLeaderboardEntriesAroundUserAsync: function(leaderboardId, countAhead, countBehind, friendsOnly) {
        return WavedashJs.callPromise("listLeaderboardEntriesAroundUser", [UTF8ToString(leaderboardId), countAhead, countBehind, WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_ListLeaderboardEntriesAsync: function(leaderboardId, offset, limit, friendsOnly) {
        return WavedashJs.callPromise("listLeaderboardEntries", [UTF8ToString(leaderboardId), offset, limit, WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_UploadLeaderboardScoreAsync: function(leaderboardId, score, keepBest, ugcId, metadataJson) {
        return WavedashJs.callPromise("uploadLeaderboardScore", [UTF8ToString(leaderboardId), score, !!keepBest, WavedashJs.optionalString(ugcId), WavedashJs.optionalJson(metadataJson)]);
    },

    WavedashJs_CreateUGCItemAsync: function(ugcType, title, description, visibility, filePath) {
        return WavedashJs.callPromise("createUGCItem", [
            ugcType,
            WavedashJs.optionalString(title),
            WavedashJs.optionalString(description),
            WavedashJs.optionalNumber(visibility),
            WavedashJs.optionalString(filePath)
        ]);
    },

    WavedashJs_UpdateUGCItemAsync: function(ugcId, title, description, visibility, filePath) {
        return WavedashJs.callPromise("updateUGCItem", [
            UTF8ToString(ugcId),
            WavedashJs.optionalString(title),
            WavedashJs.optionalString(description),
            WavedashJs.optionalNumber(visibility),
            WavedashJs.optionalString(filePath)
        ]);
    },

    WavedashJs_DownloadUGCItemAsync: function(ugcId, filePath) {
        return WavedashJs.callPromise("downloadUGCItem", [UTF8ToString(ugcId), UTF8ToString(filePath)]);
    },
    WavedashJs_DeleteUGCItemAsync: function(ugcId) {
        return WavedashJs.callPromise("deleteUGCItem", [UTF8ToString(ugcId)]);
    },
    WavedashJs_ListUGCItemsAsync: function(filters_json) {
        return WavedashJs.callPromise("listUGCItems", [WavedashJs.optionalJson(filters_json)]);
    },

    WavedashJs_DeleteRemoteFileAsync: function(filePath) {
        return WavedashJs.callPromise("deleteRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_DownloadRemoteFileAsync: function(filePath) {
        return WavedashJs.callPromise("downloadRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_UploadRemoteFileAsync: function(filePath) {
        return WavedashJs.callPromise("uploadRemoteFile", [UTF8ToString(filePath)]);
    },

    WavedashJs_ListRemoteDirectoryAsync: function(path) {
        return WavedashJs.callPromise("listRemoteDirectory", [UTF8ToString(path)]);
    },

    WavedashJs_DownloadRemoteDirectoryAsync: function(path) {
        return WavedashJs.callPromise("downloadRemoteDirectory", [UTF8ToString(path)]);
    },

    WavedashJs_WriteLocalFileAsync: function(filePath, dataPtr, dataLen) {
        return WavedashJs.callPromise("writeLocalFile", [UTF8ToString(filePath), WavedashJs.heapBytes(dataPtr, dataLen)]);
    },

    WavedashJs_ReadLocalFileAsync: function(filePath) {
        return WavedashJs.callPromise("readLocalFile", [UTF8ToString(filePath)]);
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
        return WavedashJs.callPromise("requestStats", []);
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
            WavedashJs.optionalString(toUserId),
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
        return WavedashJs.callPromise("createLobby", [visibility, WavedashJs.optionalNumber(maxPlayers)]);
    },

    WavedashJs_JoinLobbyAsync: function(lobbyId) {
        return WavedashJs.callPromise("joinLobby", [UTF8ToString(lobbyId)]);
    },

    WavedashJs_ListAvailableLobbiesAsync: function(friendsOnly) {
        return WavedashJs.callPromise("listAvailableLobbies", [WavedashJs.optionalBool(friendsOnly)]);
    },

    WavedashJs_GetLobbyUsers: function(lobbyId) {
        return WavedashJs.allocJson(WavedashJs.call("getLobbyUsers", [UTF8ToString(lobbyId)]));
    },

    WavedashJs_GetNumLobbyUsers: function(lobbyId) {
        return WavedashJs.call("getNumLobbyUsers", [UTF8ToString(lobbyId)]);
    },

    WavedashJs_GetLobbyHostId: function(lobbyId) {
        return WavedashJs.allocString(WavedashJs.call("getLobbyHostId", [UTF8ToString(lobbyId)]));
    },

    WavedashJs_GetLobbyData: function(lobbyId, key) {
        return WavedashJs.allocJson(WavedashJs.call("getLobbyData", [UTF8ToString(lobbyId), UTF8ToString(key)]));
    },

    WavedashJs_SetLobbyData: function(lobbyId, key, valueJson) {
        return WavedashJs.call("setLobbyData", [UTF8ToString(lobbyId), UTF8ToString(key), WavedashJs.optionalJson(valueJson)]) ? 1 : 0;
    },

    WavedashJs_DeleteLobbyData: function(lobbyId, key) {
        return WavedashJs.call("deleteLobbyData", [UTF8ToString(lobbyId), UTF8ToString(key)]) ? 1 : 0;
    },

    WavedashJs_LeaveLobbyAsync: function(lobbyId) {
        return WavedashJs.callPromise("leaveLobby", [UTF8ToString(lobbyId)]);
    },

    WavedashJs_SendLobbyMessage: function(lobbyId, message) {
        return WavedashJs.call("sendLobbyMessage", [UTF8ToString(lobbyId), UTF8ToString(message)]) ? 1 : 0;
    },

    WavedashJs_InviteUserToLobbyAsync: function(lobbyId, userId) {
        return WavedashJs.callPromise("inviteUserToLobby", [UTF8ToString(lobbyId), UTF8ToString(userId)]);
    },

    WavedashJs_GetLobbyInviteLinkAsync: function(copyToClipboard) {
        return WavedashJs.callPromise("getLobbyInviteLink", [WavedashJs.optionalBool(copyToClipboard)]);
    },

    WavedashJs_UpdateUserPresenceAsync: function(dataJson) {
        return WavedashJs.callPromise("updateUserPresence", [WavedashJs.optionalJson(dataJson)]);
    },

    WavedashJs_EnsureGameplayJwtAsync: function() {
        return WavedashJs.callPromise("ensureGameplayJwt", []);
    },

    WavedashJs_IsEntitledAsync: function(contentIdentifier) {
        return WavedashJs.callPromise("isEntitled", [UTF8ToString(contentIdentifier)]);
    },

    WavedashJs_GetEntitlementsAsync: function() {
        return WavedashJs.callPromise("getEntitlements", []);
    },

    WavedashJs_TriggerPaywallAsync: function(contentIdentifier) {
        return WavedashJs.callPromise("triggerPaywall", [UTF8ToString(contentIdentifier)]);
    }
}

autoAddDeps(LibWavedash, '$WavedashJs');
addToLibrary(LibWavedash);

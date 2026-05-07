local M = {}

local function lua_to_js(arg_value, arg_type)
	if not arg_value then
		return ""
	end
	
	if not arg_type then
		arg_type = type(arg_value)
	end
	
	if arg_type == "number" then
		return tostring(arg_value)
	elseif arg_type == "string" then
		return "\"" .. arg_value .. "\""
	elseif arg_type == "Uint8Array" then
		return "Uint8Array.fromBase64(\"" .. arg_value .. "\")"
	elseif type(arg_value) == "table" then
		return "'" .. json.encode(arg_value) .. "'"
	else
		return tostring(arg_value)
	end
end

local CLASS_LOOKUP = {
	["WavedashManager"] = "Wavedash.wavedashManager",
	["LobbyManager"] = "Wavedash.lobbyManager",
	["FileSystemManager"] = "Wavedash.fileSystemManager",
	["UGCManager"] = "Wavedash.ugcManager",
	["LeaderboardManager"] = "Wavedash.leaderboardManager",
	["P2PManager"] = "Wavedash.p2pManager",
	["StatsManager"] = "Wavedash.statsManager",
	["HeartbeatManager"] = "Wavedash.heartbeatManager",
	["GameEventManager"] = "Wavedash.gameEventManager",
	["FullscreenManager"] = "Wavedash.fullscreenManager",
	["OverlayManager"] = "Wavedash.overlayManager",
	["FriendsManager"] = "Wavedash.friendsManager",
	["WavedashLogger"] = "Wavedash.wavedashLogger",
	["IFrameMessenger"] = "Wavedash.iframeMessenger",
	["WavedashSDK"] = "Wavedash",
	["DefoldWavedash"] = "DefoldWavedash",
}

-- call wavedash api method using html5.run() which basically evaluates
-- a string of JS code and returns any output from the evaluation
local function run(class, method, args)
	print("run", class, method, args)
	local code = "window.Wavedash && window." .. CLASS_LOOKUP[class] .. "." .. method .. "(" .. args .. ")"
	print("run", code)
	return html5 and html5.run(code) or false
end


M.WavedashEvents = {}
M.WavedashEvents.LOBBY_MESSAGE = "LobbyMessage"
M.WavedashEvents.LOBBY_JOINED = "LobbyJoined"
M.WavedashEvents.LOBBY_KICKED = "LobbyKicked"
M.WavedashEvents.LOBBY_USERS_UPDATED = "LobbyUsersUpdated"
M.WavedashEvents.LOBBY_DATA_UPDATED = "LobbyDataUpdated"
M.WavedashEvents.LOBBY_INVITE = "LobbyInvite"
M.WavedashEvents.P2P_CONNECTION_ESTABLISHED = "P2PConnectionEstablished"
M.WavedashEvents.P2P_CONNECTION_FAILED = "P2PConnectionFailed"
M.WavedashEvents.P2P_PEER_DISCONNECTED = "P2PPeerDisconnected"
M.WavedashEvents.P2P_PEER_RECONNECTING = "P2PPeerReconnecting"
M.WavedashEvents.P2P_PEER_RECONNECTED = "P2PPeerReconnected"
M.WavedashEvents.P2P_PACKET_DROPPED = "P2PPacketDropped"
M.WavedashEvents.STATS_STORED = "StatsStored"
M.WavedashEvents.BACKEND_CONNECTED = "BackendConnected"
M.WavedashEvents.BACKEND_DISCONNECTED = "BackendDisconnected"
M.WavedashEvents.BACKEND_RECONNECTING = "BackendReconnecting"
M.WavedashEvents.FULLSCREEN_CHANGED = "FullscreenChanged"

M.LobbyKickedReason = {}
M.LobbyKickedReason.KICKED = "KICKED"
M.LobbyKickedReason.ERROR = "ERROR"

M.LobbyUserChangeType = {}
M.LobbyUserChangeType.JOINED = "JOINED"
M.LobbyUserChangeType.LEFT = "LEFT"

M.P = {}
M.P.QUEUE_FULL = "QUEUE_FULL"
M.P.PAYLOAD_TOO_LARGE = "PAYLOAD_TOO_LARGE"
M.P.INVALID_PAYLOAD_SIZE = "INVALID_PAYLOAD_SIZE"
M.P.INVALID_CHANNEL = "INVALID_CHANNEL"
M.P.MALFORMED = "MALFORMED"
M.P.PEER_NOT_READY = "PEER_NOT_READY"




M.WavedashManager = {}
M.WavedashManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("WavedashManager", "constructor", args)
end
M.WavedashManager.destroy = function()
	local args = ""
	return run("WavedashManager", "destroy", args)
end

M.LobbyManager = {}
M.LobbyManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("LobbyManager", "constructor", args)
end
M.LobbyManager.createLobby = function(visibility,maxPlayers)
	local args = ""
	args = args .. lua_to_js(visibility, "LobbyVisibility") .. ","
	args = args .. lua_to_js(maxPlayers, "number")
	return run("LobbyManager", "createLobby", args)
end
M.LobbyManager.joinLobby = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "joinLobby", args)
end
M.LobbyManager.getLobbyUsers = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "getLobbyUsers", args)
end
M.LobbyManager.getHostId = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "getHostId", args)
end
M.LobbyManager.getLobbyData = function(lobbyId,key)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string")
	return run("LobbyManager", "getLobbyData", args)
end
M.LobbyManager.deleteLobbyData = function(lobbyId,key)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string")
	return run("LobbyManager", "deleteLobbyData", args)
end
M.LobbyManager.setLobbyData = function(lobbyId,key,value)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string") .. ","
	args = args .. lua_to_js(value, "string | number | null")
	return run("LobbyManager", "setLobbyData", args)
end
M.LobbyManager.getLobbyMaxPlayers = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "getLobbyMaxPlayers", args)
end
M.LobbyManager.getNumLobbyUsers = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "getNumLobbyUsers", args)
end
M.LobbyManager.leaveLobby = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("LobbyManager", "leaveLobby", args)
end
M.LobbyManager.listAvailableLobbies = function(friendsOnly)
	local args = ""
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("LobbyManager", "listAvailableLobbies", args)
end
M.LobbyManager.sendLobbyMessage = function(lobbyId,message)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(message, "string")
	return run("LobbyManager", "sendLobbyMessage", args)
end
M.LobbyManager.inviteUserToLobby = function(lobbyId,userId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(userId, "number")
	return run("LobbyManager", "inviteUserToLobby", args)
end
M.LobbyManager.getLobbyInviteLink = function(copyToClipboard)
	local args = ""
	args = args .. lua_to_js(copyToClipboard, "boolean")
	return run("LobbyManager", "getLobbyInviteLink", args)
end
M.LobbyManager.unsubscribeFromCurrentLobby = function()
	local args = ""
	return run("LobbyManager", "unsubscribeFromCurrentLobby", args)
end
M.LobbyManager.destroy = function()
	local args = ""
	return run("LobbyManager", "destroy", args)
end

M.FileSystemManager = {}
M.FileSystemManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("FileSystemManager", "constructor", args)
end
M.FileSystemManager.uploadRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "uploadRemoteFile", args)
end
M.FileSystemManager.deleteRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "deleteRemoteFile", args)
end
M.FileSystemManager.downloadRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "downloadRemoteFile", args)
end
M.FileSystemManager.listRemoteDirectory = function(path)
	local args = ""
	args = args .. lua_to_js(path, "string")
	return run("FileSystemManager", "listRemoteDirectory", args)
end
M.FileSystemManager.downloadRemoteDirectory = function(path)
	local args = ""
	args = args .. lua_to_js(path, "string")
	return run("FileSystemManager", "downloadRemoteDirectory", args)
end
M.FileSystemManager.writeLocalFile = function(filePath,data)
	local args = ""
	args = args .. lua_to_js(filePath, "string") .. ","
	args = args .. lua_to_js(data, "Uint8Array")
	return run("FileSystemManager", "writeLocalFile", args)
end
M.FileSystemManager.readLocalFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "readLocalFile", args)
end
M.FileSystemManager.upload = function(presignedUploadUrl,filePath)
	local args = ""
	args = args .. lua_to_js(presignedUploadUrl, "string") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "upload", args)
end
M.FileSystemManager.download = function(url,filePath)
	local args = ""
	args = args .. lua_to_js(url, "string") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("FileSystemManager", "download", args)
end

M.UGCManager = {}
M.UGCManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("UGCManager", "constructor", args)
end
M.UGCManager.createUGCItem = function(ugcType,title,description,visibility,filePath)
	local args = ""
	args = args .. lua_to_js(ugcType, "UGCType") .. ","
	args = args .. lua_to_js(title, "string") .. ","
	args = args .. lua_to_js(description, "string") .. ","
	args = args .. lua_to_js(visibility, "UGCVisibility") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("UGCManager", "createUGCItem", args)
end
M.UGCManager.updateUGCItem = function(ugcId,title,description,visibility,filePath)
	local args = ""
	args = args .. lua_to_js(ugcId, "number") .. ","
	args = args .. lua_to_js(title, "string") .. ","
	args = args .. lua_to_js(description, "string") .. ","
	args = args .. lua_to_js(visibility, "UGCVisibility") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("UGCManager", "updateUGCItem", args)
end
M.UGCManager.downloadUGCItem = function(ugcId,filePath)
	local args = ""
	args = args .. lua_to_js(ugcId, "number") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("UGCManager", "downloadUGCItem", args)
end

M.LeaderboardManager = {}
M.LeaderboardManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("LeaderboardManager", "constructor", args)
end
M.LeaderboardManager.getLeaderboard = function(name)
	local args = ""
	args = args .. lua_to_js(name, "string")
	return run("LeaderboardManager", "getLeaderboard", args)
end
M.LeaderboardManager.getOrCreateLeaderboard = function(name,sortOrder,displayType)
	local args = ""
	args = args .. lua_to_js(name, "string") .. ","
	args = args .. lua_to_js(sortOrder, "LeaderboardSortOrder") .. ","
	args = args .. lua_to_js(displayType, "LeaderboardDisplayType")
	return run("LeaderboardManager", "getOrCreateLeaderboard", args)
end
M.LeaderboardManager.getLeaderboardEntryCount = function(leaderboardId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number")
	return run("LeaderboardManager", "getLeaderboardEntryCount", args)
end
M.LeaderboardManager.getMyLeaderboardEntries = function(leaderboardId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number")
	return run("LeaderboardManager", "getMyLeaderboardEntries", args)
end
M.LeaderboardManager.listLeaderboardEntriesAroundUser = function(leaderboardId,countAhead,countBehind,friendsOnly)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(countAhead, "number") .. ","
	args = args .. lua_to_js(countBehind, "number") .. ","
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("LeaderboardManager", "listLeaderboardEntriesAroundUser", args)
end
M.LeaderboardManager.listLeaderboardEntries = function(leaderboardId,offset,limit,friendsOnly)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(offset, "number") .. ","
	args = args .. lua_to_js(limit, "number") .. ","
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("LeaderboardManager", "listLeaderboardEntries", args)
end
M.LeaderboardManager.uploadLeaderboardScore = function(leaderboardId,score,keepBest,ugcId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(score, "number") .. ","
	args = args .. lua_to_js(keepBest, "boolean") .. ","
	args = args .. lua_to_js(ugcId, "number")
	return run("LeaderboardManager", "uploadLeaderboardScore", args)
end

M.P2PManager = {}
M.P2PManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("P2PManager", "constructor", args)
end
M.P2PManager.destroy = function()
	local args = ""
	return run("P2PManager", "destroy", args)
end
M.P2PManager.init = function(config)
	local args = ""
	args = args .. lua_to_js(config, "Partial<P2PConfig>")
	return run("P2PManager", "init", args)
end
M.P2PManager.initializeP2PForCurrentLobby = function(lobbyId,members)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(members, "SDKUser[]")
	return run("P2PManager", "initializeP2PForCurrentLobby", args)
end
M.P2PManager.sendP2PMessage = function(toUserId,appChannel,reliable,payload,payloadSize)
	local args = ""
	args = args .. lua_to_js(toUserId, "number") .. ","
	args = args .. lua_to_js(appChannel, "number | undefined") .. ","
	args = args .. lua_to_js(reliable, "boolean | undefined") .. ","
	args = args .. lua_to_js(payload, "Uint8Array") .. ","
	args = args .. lua_to_js(payloadSize, "number")
	return run("P2PManager", "sendP2PMessage", args)
end
M.P2PManager.disconnectP2P = function()
	local args = ""
	return run("P2PManager", "disconnectP2P", args)
end
M.P2PManager.isPeerReady = function(userId)
	local args = ""
	args = args .. lua_to_js(userId, "number")
	return run("P2PManager", "isPeerReady", args)
end
M.P2PManager.isBroadcastReady = function()
	local args = ""
	return run("P2PManager", "isBroadcastReady", args)
end
M.P2PManager.getPeerStatuses = function()
	local args = ""
	return run("P2PManager", "getPeerStatuses", args)
end
M.P2PManager.getMaxPayloadSize = function()
	local args = ""
	return run("P2PManager", "getMaxPayloadSize", args)
end
M.P2PManager.getMaxIncomingMessages = function()
	local args = ""
	return run("P2PManager", "getMaxIncomingMessages", args)
end
M.P2PManager.getOutgoingMessageBuffer = function()
	local args = ""
	return run("P2PManager", "getOutgoingMessageBuffer", args)
end
M.P2PManager.readMessageFromChannel = function(appChannel)
	local args = ""
	args = args .. lua_to_js(appChannel, "number")
	return run("P2PManager", "readMessageFromChannel", args)
end
M.P2PManager.drainChannelToBuffer = function(appChannel,buffer)
	local args = ""
	args = args .. lua_to_js(appChannel, "number") .. ","
	args = args .. lua_to_js(buffer, "Uint8Array")
	return run("P2PManager", "drainChannelToBuffer", args)
end

M.StatsManager = {}
M.StatsManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("StatsManager", "constructor", args)
end
M.StatsManager.destroy = function()
	local args = ""
	return run("StatsManager", "destroy", args)
end
M.StatsManager.requestStats = function()
	local args = ""
	return run("StatsManager", "requestStats", args)
end
M.StatsManager.storeStats = function()
	local args = ""
	return run("StatsManager", "storeStats", args)
end
M.StatsManager.getStat = function(identifier)
	local args = ""
	args = args .. lua_to_js(identifier, "string")
	return run("StatsManager", "getStat", args)
end
M.StatsManager.setStat = function(identifier,value,storeNow)
	local args = ""
	args = args .. lua_to_js(identifier, "string") .. ","
	args = args .. lua_to_js(value, "number") .. ","
	args = args .. lua_to_js(storeNow, "boolean")
	return run("StatsManager", "setStat", args)
end
M.StatsManager.getAchievement = function(identifier)
	local args = ""
	args = args .. lua_to_js(identifier, "string")
	return run("StatsManager", "getAchievement", args)
end
M.StatsManager.setAchievement = function(identifier,storeNow)
	local args = ""
	args = args .. lua_to_js(identifier, "string") .. ","
	args = args .. lua_to_js(storeNow, "boolean")
	return run("StatsManager", "setAchievement", args)
end
M.StatsManager.getPendingData = function()
	local args = ""
	return run("StatsManager", "getPendingData", args)
end

M.HeartbeatManager = {}
M.HeartbeatManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("HeartbeatManager", "constructor", args)
end
M.HeartbeatManager.start = function()
	local args = ""
	return run("HeartbeatManager", "start", args)
end
M.HeartbeatManager.stop = function()
	local args = ""
	return run("HeartbeatManager", "stop", args)
end
M.HeartbeatManager.destroy = function()
	local args = ""
	return run("HeartbeatManager", "destroy", args)
end
M.HeartbeatManager.updateUserPresence = function(data)
	local args = ""
	args = args .. lua_to_js(data, "Record<string, unknown>")
	return run("HeartbeatManager", "updateUserPresence", args)
end
M.HeartbeatManager.isCurrentlyConnected = function()
	local args = ""
	return run("HeartbeatManager", "isCurrentlyConnected", args)
end

M.GameEventManager = {}
M.GameEventManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("GameEventManager", "constructor", args)
end
M.GameEventManager.notifyGame = function(event,payload)
	local args = ""
	args = args .. lua_to_js(event, "WavedashEvent") .. ","
	args = args .. lua_to_js(payload, "string | number | object")
	return run("GameEventManager", "notifyGame", args)
end
M.GameEventManager.flushEventQueue = function()
	local args = ""
	return run("GameEventManager", "flushEventQueue", args)
end

M.FullscreenManager = {}
M.FullscreenManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("FullscreenManager", "constructor", args)
end
M.FullscreenManager.isFullscreen = function()
	local args = ""
	return run("FullscreenManager", "isFullscreen", args)
end
M.FullscreenManager.requestFullscreen = function(fullscreen)
	local args = ""
	args = args .. lua_to_js(fullscreen, "boolean")
	return run("FullscreenManager", "requestFullscreen", args)
end
M.FullscreenManager.toggleFullscreen = function()
	local args = ""
	return run("FullscreenManager", "toggleFullscreen", args)
end
M.FullscreenManager.subscribe = function(listener)
	local args = ""
	args = args .. lua_to_js(listener, "(isFullscreen: boolean)")
	return run("FullscreenManager", "subscribe", args)
end

M.OverlayManager = {}
M.OverlayManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("OverlayManager", "constructor", args)
end
M.OverlayManager.toggleOverlay = function()
	local args = ""
	return run("OverlayManager", "toggleOverlay", args)
end
M.OverlayManager.takeFocus = function()
	local args = ""
	return run("OverlayManager", "takeFocus", args)
end

M.FriendsManager = {}
M.FriendsManager.constructor = function(sdk)
	local args = ""
	args = args .. lua_to_js(sdk, "WavedashSDK")
	return run("FriendsManager", "constructor", args)
end
M.FriendsManager.cacheUsers = function(users)
	local args = ""
	args = args .. lua_to_js(users, "array")
	return run("FriendsManager", "cacheUsers", args)
end
M.FriendsManager.getUserAvatarUrl = function(userId,size)
	local args = ""
	args = args .. lua_to_js(userId, "number") .. ","
	args = args .. lua_to_js(size, "number")
	return run("FriendsManager", "getUserAvatarUrl", args)
end
M.FriendsManager.getUsername = function(userId)
	local args = ""
	args = args .. lua_to_js(userId, "number")
	return run("FriendsManager", "getUsername", args)
end
M.FriendsManager.listFriends = function()
	local args = ""
	return run("FriendsManager", "listFriends", args)
end

M.WavedashLogger = {}
M.WavedashLogger.constructor = function(logLevel)
	local args = ""
	args = args .. lua_to_js(logLevel, "number")
	return run("WavedashLogger", "constructor", args)
end
M.WavedashLogger.setLogLevel = function(level)
	local args = ""
	args = args .. lua_to_js(level, "number")
	return run("WavedashLogger", "setLogLevel", args)
end
M.WavedashLogger.debug = function(message,args)
	local args = ""
	args = args .. lua_to_js(message, "string") .. ","
	args = args .. lua_to_js(args, "unknown[]")
	return run("WavedashLogger", "debug", args)
end
M.WavedashLogger.info = function(message,args)
	local args = ""
	args = args .. lua_to_js(message, "string") .. ","
	args = args .. lua_to_js(args, "unknown[]")
	return run("WavedashLogger", "info", args)
end
M.WavedashLogger.warn = function(message,args)
	local args = ""
	args = args .. lua_to_js(message, "string") .. ","
	args = args .. lua_to_js(args, "unknown[]")
	return run("WavedashLogger", "warn", args)
end
M.WavedashLogger.error = function(message,args)
	local args = ""
	args = args .. lua_to_js(message, "string") .. ","
	args = args .. lua_to_js(args, "unknown[]")
	return run("WavedashLogger", "error", args)
end

M.IFrameMessenger = {}
M.IFrameMessenger.constructor = function()
	local args = ""
	return run("IFrameMessenger", "constructor", args)
end
M.IFrameMessenger.addEventListener = function(type,listener)
	local args = ""
	args = args .. lua_to_js(type, "T") .. ","
	args = args .. lua_to_js(listener, "PushListener<T>")
	return run("IFrameMessenger", "addEventListener", args)
end
M.IFrameMessenger.removeEventListener = function(type,listener)
	local args = ""
	args = args .. lua_to_js(type, "T") .. ","
	args = args .. lua_to_js(listener, "PushListener<T>")
	return run("IFrameMessenger", "removeEventListener", args)
end
M.IFrameMessenger.postToParent = function(requestType,data)
	local args = ""
	args = args .. lua_to_js(requestType, "(typeof IFRAME_MESSAGE_TYPE)[keyof typeof IFRAME_MESSAGE_TYPE]") .. ","
	args = args .. lua_to_js(data, "Record<string, string | number | boolean>")
	return run("IFrameMessenger", "postToParent", args)
end
M.IFrameMessenger.requestFromParent = function(requestType,data)
	local args = ""
	args = args .. lua_to_js(requestType, "T") .. ","
	args = args .. lua_to_js(data, "Record<string, unknown>")
	return run("IFrameMessenger", "requestFromParent", args)
end

M.WavedashSDK = {}
M.WavedashSDK.constructor = function(sdkConfig)
	local args = ""
	args = args .. lua_to_js(sdkConfig, "SDKConfig")
	return run("WavedashSDK", "constructor", args)
end
M.WavedashSDK.init = function(config)
	local args = ""
	args = args .. lua_to_js(config, "WavedashConfig")
	return run("WavedashSDK", "init", args)
end
M.WavedashSDK.readyForEvents = function()
	local args = ""
	return run("WavedashSDK", "readyForEvents", args)
end
M.WavedashSDK.on = function(event,listener)
	local args = ""
	args = args .. lua_to_js(event, "K") .. ","
	args = args .. lua_to_js(listener, "(payload: WavedashEventMap[K])")
	return run("WavedashSDK", "on", args)
end
M.WavedashSDK.off = function(event,listener)
	local args = ""
	args = args .. lua_to_js(event, "K") .. ","
	args = args .. lua_to_js(listener, "(payload: WavedashEventMap[K])")
	return run("WavedashSDK", "off", args)
end
M.WavedashSDK.addEventListener = function(type,listener,options)
	local args = ""
	args = args .. lua_to_js(type, "K") .. ","
	args = args .. lua_to_js(listener, "(ev: CustomEvent<WavedashEventMap[K]>)") .. ","
	args = args .. lua_to_js(options, "boolean | AddEventListenerOptions")
	return run("WavedashSDK", "addEventListener", args)
end
M.WavedashSDK.addEventListener = function(type,listener,options)
	local args = ""
	args = args .. lua_to_js(type, "string") .. ","
	args = args .. lua_to_js(listener, "EventListenerOrEventListenerObject | null") .. ","
	args = args .. lua_to_js(options, "boolean | AddEventListenerOptions")
	return run("WavedashSDK", "addEventListener", args)
end
M.WavedashSDK.removeEventListener = function(type,listener,options)
	local args = ""
	args = args .. lua_to_js(type, "K") .. ","
	args = args .. lua_to_js(listener, "(ev: CustomEvent<WavedashEventMap[K]>)") .. ","
	args = args .. lua_to_js(options, "boolean | EventListenerOptions")
	return run("WavedashSDK", "removeEventListener", args)
end
M.WavedashSDK.removeEventListener = function(type,listener,options)
	local args = ""
	args = args .. lua_to_js(type, "string") .. ","
	args = args .. lua_to_js(listener, "EventListenerOrEventListenerObject | null") .. ","
	args = args .. lua_to_js(options, "boolean | EventListenerOptions")
	return run("WavedashSDK", "removeEventListener", args)
end
M.WavedashSDK.loadScript = function(src)
	local args = ""
	args = args .. lua_to_js(src, "string")
	return run("WavedashSDK", "loadScript", args)
end
M.WavedashSDK.updateLoadProgressZeroToOne = function(progress)
	local args = ""
	args = args .. lua_to_js(progress, "number")
	return run("WavedashSDK", "updateLoadProgressZeroToOne", args)
end
M.WavedashSDK.loadComplete = function()
	local args = ""
	return run("WavedashSDK", "loadComplete", args)
end
M.WavedashSDK.toggleOverlay = function()
	local args = ""
	return run("WavedashSDK", "toggleOverlay", args)
end
M.WavedashSDK.isFullscreen = function()
	local args = ""
	return run("WavedashSDK", "isFullscreen", args)
end
M.WavedashSDK.requestFullscreen = function(fullscreen)
	local args = ""
	args = args .. lua_to_js(fullscreen, "boolean")
	return run("WavedashSDK", "requestFullscreen", args)
end
M.WavedashSDK.toggleFullscreen = function()
	local args = ""
	return run("WavedashSDK", "toggleFullscreen", args)
end
M.WavedashSDK.getUser = function()
	local args = ""
	return run("WavedashSDK", "getUser", args)
end
M.WavedashSDK.getUsername = function()
	local args = ""
	return run("WavedashSDK", "getUsername", args)
end
M.WavedashSDK.getUsername = function(userId)
	local args = ""
	args = args .. lua_to_js(userId, "number")
	return run("WavedashSDK", "getUsername", args)
end
M.WavedashSDK.getUserId = function()
	local args = ""
	return run("WavedashSDK", "getUserId", args)
end
M.WavedashSDK.getUserJwt = function()
	local args = ""
	return run("WavedashSDK", "getUserJwt", args)
end
M.WavedashSDK.getLaunchParams = function()
	local args = ""
	return run("WavedashSDK", "getLaunchParams", args)
end
M.WavedashSDK.listFriends = function()
	local args = ""
	return run("WavedashSDK", "listFriends", args)
end
M.WavedashSDK.getUserAvatarUrl = function(userId,size)
	local args = ""
	args = args .. lua_to_js(userId, "number") .. ","
	args = args .. lua_to_js(size, "number")
	return run("WavedashSDK", "getUserAvatarUrl", args)
end
M.WavedashSDK.getLeaderboard = function(name)
	local args = ""
	args = args .. lua_to_js(name, "string")
	return run("WavedashSDK", "getLeaderboard", args)
end
M.WavedashSDK.getOrCreateLeaderboard = function(name,sortOrder,displayType)
	local args = ""
	args = args .. lua_to_js(name, "string") .. ","
	args = args .. lua_to_js(sortOrder, "LeaderboardSortOrder") .. ","
	args = args .. lua_to_js(displayType, "LeaderboardDisplayType")
	return run("WavedashSDK", "getOrCreateLeaderboard", args)
end
M.WavedashSDK.getLeaderboardEntryCount = function(leaderboardId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number")
	return run("WavedashSDK", "getLeaderboardEntryCount", args)
end
M.WavedashSDK.getMyLeaderboardEntries = function(leaderboardId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number")
	return run("WavedashSDK", "getMyLeaderboardEntries", args)
end
M.WavedashSDK.listLeaderboardEntriesAroundUser = function(leaderboardId,countAhead,countBehind,friendsOnly)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(countAhead, "number") .. ","
	args = args .. lua_to_js(countBehind, "number") .. ","
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("WavedashSDK", "listLeaderboardEntriesAroundUser", args)
end
M.WavedashSDK.listLeaderboardEntries = function(leaderboardId,offset,limit,friendsOnly)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(offset, "number") .. ","
	args = args .. lua_to_js(limit, "number") .. ","
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("WavedashSDK", "listLeaderboardEntries", args)
end
M.WavedashSDK.uploadLeaderboardScore = function(leaderboardId,score,keepBest,ugcId)
	local args = ""
	args = args .. lua_to_js(leaderboardId, "number") .. ","
	args = args .. lua_to_js(score, "number") .. ","
	args = args .. lua_to_js(keepBest, "boolean") .. ","
	args = args .. lua_to_js(ugcId, "number")
	return run("WavedashSDK", "uploadLeaderboardScore", args)
end
M.WavedashSDK.createUGCItem = function(ugcType,title,description,visibility,filePath)
	local args = ""
	args = args .. lua_to_js(ugcType, "UGCType") .. ","
	args = args .. lua_to_js(title, "string") .. ","
	args = args .. lua_to_js(description, "string") .. ","
	args = args .. lua_to_js(visibility, "UGCVisibility") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "createUGCItem", args)
end
M.WavedashSDK.updateUGCItem = function(ugcId,title,description,visibility,filePath)
	local args = ""
	args = args .. lua_to_js(ugcId, "number") .. ","
	args = args .. lua_to_js(title, "string") .. ","
	args = args .. lua_to_js(description, "string") .. ","
	args = args .. lua_to_js(visibility, "UGCVisibility") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "updateUGCItem", args)
end
M.WavedashSDK.downloadUGCItem = function(ugcId,filePath)
	local args = ""
	args = args .. lua_to_js(ugcId, "number") .. ","
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "downloadUGCItem", args)
end
M.WavedashSDK.deleteRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "deleteRemoteFile", args)
end
M.WavedashSDK.downloadRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "downloadRemoteFile", args)
end
M.WavedashSDK.uploadRemoteFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "uploadRemoteFile", args)
end
M.WavedashSDK.listRemoteDirectory = function(path)
	local args = ""
	args = args .. lua_to_js(path, "string")
	return run("WavedashSDK", "listRemoteDirectory", args)
end
M.WavedashSDK.downloadRemoteDirectory = function(path)
	local args = ""
	args = args .. lua_to_js(path, "string")
	return run("WavedashSDK", "downloadRemoteDirectory", args)
end
M.WavedashSDK.writeLocalFile = function(filePath,data)
	local args = ""
	args = args .. lua_to_js(filePath, "string") .. ","
	args = args .. lua_to_js(data, "Uint8Array")
	return run("WavedashSDK", "writeLocalFile", args)
end
M.WavedashSDK.readLocalFile = function(filePath)
	local args = ""
	args = args .. lua_to_js(filePath, "string")
	return run("WavedashSDK", "readLocalFile", args)
end
M.WavedashSDK.getAchievement = function(identifier)
	local args = ""
	args = args .. lua_to_js(identifier, "string")
	return run("WavedashSDK", "getAchievement", args)
end
M.WavedashSDK.getStat = function(identifier)
	local args = ""
	args = args .. lua_to_js(identifier, "string")
	return run("WavedashSDK", "getStat", args)
end
M.WavedashSDK.setAchievement = function(identifier,storeNow)
	local args = ""
	args = args .. lua_to_js(identifier, "string") .. ","
	args = args .. lua_to_js(storeNow, "boolean")
	return run("WavedashSDK", "setAchievement", args)
end
M.WavedashSDK.setStat = function(identifier,value,storeNow)
	local args = ""
	args = args .. lua_to_js(identifier, "string") .. ","
	args = args .. lua_to_js(value, "number") .. ","
	args = args .. lua_to_js(storeNow, "boolean")
	return run("WavedashSDK", "setStat", args)
end
M.WavedashSDK.requestStats = function()
	local args = ""
	return run("WavedashSDK", "requestStats", args)
end
M.WavedashSDK.storeStats = function()
	local args = ""
	return run("WavedashSDK", "storeStats", args)
end
M.WavedashSDK.getP2PMaxPayloadSize = function()
	local args = ""
	return run("WavedashSDK", "getP2PMaxPayloadSize", args)
end
M.WavedashSDK.getP2PMaxIncomingMessages = function()
	local args = ""
	return run("WavedashSDK", "getP2PMaxIncomingMessages", args)
end
M.WavedashSDK.getP2POutgoingMessageBuffer = function()
	local args = ""
	return run("WavedashSDK", "getP2POutgoingMessageBuffer", args)
end
M.WavedashSDK.sendP2PMessage = function(toUserId,appChannel,reliable,payload,payloadSize)
	local args = ""
	args = args .. lua_to_js(toUserId, "number") .. ","
	args = args .. lua_to_js(appChannel, "number | undefined") .. ","
	args = args .. lua_to_js(reliable, "boolean | undefined") .. ","
	args = args .. lua_to_js(payload, "Uint8Array") .. ","
	args = args .. lua_to_js(payloadSize, "number")
	return run("WavedashSDK", "sendP2PMessage", args)
end
M.WavedashSDK.broadcastP2PMessage = function(appChannel,reliable,payload,payloadSize)
	local args = ""
	args = args .. lua_to_js(appChannel, "number | undefined") .. ","
	args = args .. lua_to_js(reliable, "boolean | undefined") .. ","
	args = args .. lua_to_js(payload, "Uint8Array") .. ","
	args = args .. lua_to_js(payloadSize, "number")
	return run("WavedashSDK", "broadcastP2PMessage", args)
end
M.WavedashSDK.readP2PMessageFromChannel = function(appChannel)
	local args = ""
	args = args .. lua_to_js(appChannel, "number")
	return run("WavedashSDK", "readP2PMessageFromChannel", args)
end
M.WavedashSDK.drainP2PChannelToBuffer = function(appChannel,buffer)
	local args = ""
	args = args .. lua_to_js(appChannel, "number") .. ","
	args = args .. lua_to_js(buffer, "Uint8Array")
	return run("WavedashSDK", "drainP2PChannelToBuffer", args)
end
M.WavedashSDK.createLobby = function(visibility,maxPlayers)
	local args = ""
	args = args .. lua_to_js(visibility, "LobbyVisibility") .. ","
	args = args .. lua_to_js(maxPlayers, "number")
	return run("WavedashSDK", "createLobby", args)
end
M.WavedashSDK.joinLobby = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("WavedashSDK", "joinLobby", args)
end
M.WavedashSDK.listAvailableLobbies = function(friendsOnly)
	local args = ""
	args = args .. lua_to_js(friendsOnly, "boolean")
	return run("WavedashSDK", "listAvailableLobbies", args)
end
M.WavedashSDK.getLobbyUsers = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("WavedashSDK", "getLobbyUsers", args)
end
M.WavedashSDK.getNumLobbyUsers = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("WavedashSDK", "getNumLobbyUsers", args)
end
M.WavedashSDK.getLobbyHostId = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("WavedashSDK", "getLobbyHostId", args)
end
M.WavedashSDK.getLobbyData = function(lobbyId,key)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string")
	return run("WavedashSDK", "getLobbyData", args)
end
M.WavedashSDK.setLobbyData = function(lobbyId,key,value)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string") .. ","
	args = args .. lua_to_js(value, "string | number | null")
	return run("WavedashSDK", "setLobbyData", args)
end
M.WavedashSDK.deleteLobbyData = function(lobbyId,key)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(key, "string")
	return run("WavedashSDK", "deleteLobbyData", args)
end
M.WavedashSDK.leaveLobby = function(lobbyId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number")
	return run("WavedashSDK", "leaveLobby", args)
end
M.WavedashSDK.sendLobbyMessage = function(lobbyId,message)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(message, "string")
	return run("WavedashSDK", "sendLobbyMessage", args)
end
M.WavedashSDK.inviteUserToLobby = function(lobbyId,userId)
	local args = ""
	args = args .. lua_to_js(lobbyId, "number") .. ","
	args = args .. lua_to_js(userId, "number")
	return run("WavedashSDK", "inviteUserToLobby", args)
end
M.WavedashSDK.getLobbyInviteLink = function(copyToClipboard)
	local args = ""
	args = args .. lua_to_js(copyToClipboard, "boolean")
	return run("WavedashSDK", "getLobbyInviteLink", args)
end
M.WavedashSDK.updateUserPresence = function(data)
	local args = ""
	args = args .. lua_to_js(data, "Record<string, unknown>")
	return run("WavedashSDK", "updateUserPresence", args)
end
M.WavedashSDK.ensureGameplayJwt = function()
	local args = ""
	return run("WavedashSDK", "ensureGameplayJwt", args)
end



local event_queue = {}

M.WavedashSDK.on = function(event, cb)
	print("on", event, cb)
	if not event_queue[event] then
		event_queue[event] = {
			events = {},
			listeners = {},
		}
	end

	local events = event_queue[event].events
	local listeners = event_queue[event].listeners
	listeners[#listeners + 1] = cb


	local handle = timer.delay(1, false, function()
		local result = run("DefoldWavedash", "get_event", lua_to_js(event, "string"))
		print("timer", event, result)
	end)
	-- events[event][cb] = {
	-- 	timerh = handle
	-- }

	run("DefoldWavedash", "on", lua_to_js(event, "string"))
end

return M
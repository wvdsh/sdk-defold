#define LIB_NAME "Wavedash"
#define MODULE_NAME "wavedash"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN "wavedash"
#endif

#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_HTML5)

extern "C" {
    void WavedashJs_Init();
}

static int Wavedash_Init(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    WavedashJs_Init();
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"init", Wavedash_Init},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    luaL_register(L, MODULE_NAME, Module_methods);

    SETCONSTANT(COMMERCIAL_BREAK_SUCCESS, COMMERCIAL_BREAK_SUCCESS);
    SETCONSTANT(COMMERCIAL_BREAK_START, COMMERCIAL_BREAK_START);

    SETCONSTANT(REWARDED_BREAK_ERROR, REWARDED_BREAK_ERROR);
    SETCONSTANT(REWARDED_BREAK_SUCCESS, REWARDED_BREAK_SUCCESS);
    SETCONSTANT(REWARDED_BREAK_START, REWARDED_BREAK_START);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeWavedash(dmExtension::AppParams* params)
{
	dmLogWarning("Registered %s (null) Extension", MODULE_NAME);
	return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeWavedash(dmExtension::Params* params)
{
	LuaInit(params->m_L);
	return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeWavedash(dmExtension::AppParams* params)
{
	return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeWavedash(dmExtension::Params* params)
{
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(wavedash, LIB_NAME, AppInitializeWavedash, AppFinalizeWavedash, InitializeWavedash, 0, 0, FinalizeWavedash)

#endif

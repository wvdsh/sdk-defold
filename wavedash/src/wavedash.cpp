#define LIB_NAME "Wavedash"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN "wavedash"
#endif

#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_HTML5)

#include "wavedash_api.h"

static dmExtension::Result AppInitializeWavedash(dmExtension::AppParams* params)
{
	return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeWavedash(dmExtension::Params* params)
{
	WavedashLuaInit(params->m_L);
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

#include <DatafilePluginsdk.h>

#ifdef BNSKR
#include <KR/BnsTableNames.h>
#define PLUGIN_DETOUR_GUARD_REGION(params) \
        PLUGIN_DETOUR_GUARD(params, BnsTables::KR::TableNames::GetTableVersion)
#else
#include <EU/BnsTableNames.h>
#define PLUGIN_DETOUR_GUARD_REGION(params) \
        PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion)
#endif
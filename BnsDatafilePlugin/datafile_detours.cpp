#include "datafile_detours.h"
#include "GlobalState.h"
#include "pluginhelpers.h"
#include <DatafilePluginsdk.h>
#include "Dynamic/item/AAA_item_RecordBase.h"


using namespace PluginGlobalState;
using namespace BnsTables::Dynamic;

/**
 * @brief Example detour function for the "item" table.
 *
 * This function demonstrates how to intercept lookups for items in the datafile.
 * Plugin authors can modify or redirect lookups as needed.
 *
 * @param params Parameters for the plugin execution, including the table and key.
 * @return PluginReturnData Optionally returns a replacement element. Only return if you absolutely have to. Prefer modifying the element in place.
 */
PluginReturnData __fastcall DatafileItemDetour(PluginExecuteParams* params) {
	PLUGIN_DETOUR_GUARD_REGION(params);

	if (!g_pluginConfig.configData.enabled || !g_isReady.load(std::memory_order_acquire)) {
		return {};
	}
	unsigned __int64 key = params->key;

	// Example to replace one item with another when the game looks it up

	if (key == 4295877296) {
		DrEl* result = params->oFind(params->table, 4295917336);
		item_Record* itemRecord = (item_Record*)result; //cast to region specific item Record
		//params->displaySystemChatMessage(L"ExampleItemPlugin: Redirected item key 4295902840 to 4294967396", false);
		return { result };
	}

	return {};
}
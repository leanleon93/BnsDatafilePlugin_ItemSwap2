#include "datafile_detours.h"
#include "GlobalState.h"
#include "pluginhelpers.h"
#include <DatafilePluginsdk.h>
#include "Dynamic/item/AAA_item_RecordBase.h"
#include "Dynamic/item/item_costume_Record.h"


using namespace PluginGlobalState;
using namespace BnsTables::Dynamic;


//zulia special
static constexpr const wchar_t* eeFemaleMesh = L"/Game/neo_art/art/character/cloth/65071-금각은각조종자/jinf/mesh/65071_jinf.65071_jinf";
static constexpr const wchar_t* eeFemaleCol1 = L"/Game/neo_art/art/character/cloth/65071-금각은각조종자/jinf/col1/col1.col1";
static constexpr const wchar_t* eeFemaleCol2 = L"";

//lancer special
static constexpr const wchar_t* ee2FemaleMesh = L"/Game/Neo_Art/Art/Character/Cloth/19169-25Neo참격사직업의상/JinF/Mesh/19169_JinF.19169_JinF";
static constexpr const wchar_t* ee2FemaleCol1 = L"/Game/Art/Resource/Character/Skin/JinF_NewSkin.JinF_NewSkin";
static constexpr const wchar_t* ee2FemaleCol2 = L"/Game/Neo_Art/Art/Character/Cloth/19169-25Neo참격사직업의상/JinF/Col1/Col1.Col1";
static constexpr const wchar_t* ee2MaleMesh = L"/Game/Neo_Art/Art/Character/Cloth/19169-25Neo참격사직업의상/JinM/Mesh/19169_JinM.19169_JinM";
static constexpr const wchar_t* ee2MaleCol1 = L"/Game/Art/Resource/Character/Skin/JinM_NewSkin.JinM_NewSkin";
static constexpr const wchar_t* ee2MaleCol2 = L"/Game/Neo_Art/Art/Character/Cloth/19169-25Neo참격사직업의상/JinM/Col1/Col1.Col1";

static int eeCopyOrigId = 0;
static int ee2CopyOrigId = 0;
static item_costume_Record* eeCopy = nullptr;
static item_costume_Record* ee2Copy = nullptr;

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

	item_Record::Key origKey{};
	origKey.key = params->key;
	item_Record::Key currentKey{};
	currentKey.key = params->key;
	const auto swapMap = g_pluginConfig.configData.GetEnabledSwapMap();
	if (swapMap.empty()) return {};
	auto it = swapMap.find(currentKey.id);
	if (it != swapMap.end()) {
		currentKey.id = it->second;
		currentKey.level = 1;
		if (currentKey.id == 69696969) { //do you like zulia?
			if (eeCopy == nullptr || eeCopyOrigId != origKey.id) {
				auto record = (item_costume_Record*)params->oFind(params->table, params->key);
				if (record != nullptr) {
					eeCopy = new item_costume_Record(*record);
					eeCopy->jin_female_mesh = const_cast<wchar_t*>(eeFemaleMesh);
					eeCopy->jin_female_mesh_col[0] = const_cast<wchar_t*>(eeFemaleCol1);
					eeCopy->jin_female_mesh_col[1] = const_cast<wchar_t*>(eeFemaleCol2);
					eeCopy->item_grade = 9;
					eeCopy->icon.IcontextureId = eeIconTextureId;
					eeCopyOrigId = origKey.id;
				}
			}
			return { (DrEl*)eeCopy };
		}
		if (currentKey.id == 69696970) { //do you like lancer?
			if (ee2Copy == nullptr || ee2CopyOrigId != origKey.id) {
				auto record = (item_costume_Record*)params->oFind(params->table, params->key);
				if (record != nullptr) {
					ee2Copy = new item_costume_Record(*record);
					ee2Copy->jin_female_mesh = const_cast<wchar_t*>(ee2FemaleMesh);
					ee2Copy->jin_female_mesh_col[0] = const_cast<wchar_t*>(ee2FemaleCol1);
					ee2Copy->jin_female_mesh_col[1] = const_cast<wchar_t*>(ee2FemaleCol2);
					ee2Copy->jin_male_mesh = const_cast<wchar_t*>(ee2MaleMesh);
					ee2Copy->jin_male_mesh_col[0] = const_cast<wchar_t*>(ee2MaleCol1);
					ee2Copy->jin_male_mesh_col[1] = const_cast<wchar_t*>(ee2MaleCol2);
					ee2Copy->item_grade = 9;
					ee2Copy->icon.IcontextureId = ee2IconTextureId;
					ee2CopyOrigId = origKey.id;
				}
			}
			return { (DrEl*)ee2Copy };
		}
		auto recordBase = params->oFind(params->table, currentKey.key);
		if (recordBase != nullptr) {
			return { recordBase };
		}
	}

	return {};
}
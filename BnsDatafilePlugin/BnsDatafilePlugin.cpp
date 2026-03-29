#include "BnsDatafilePlugin.h"
#include <thread>
#include "GlobalState.h"
#include "UiPanel.h"
#include <DatafilePluginsdk.h>
#include <imgui_plugin_api.h>
#include <pluginversion.h>
#include "datafile_detours.h"
#include <mutex>
#include "Logger.h"
#include <chrono>
#include <Dynamic/icontexture/AAA_icontexture_RecordBase.h>
#include <memory>
#include <wchar.h>

/**
 * @file BnsDatafilePlugin.cpp
 * @brief Template implementation for a datafile plugin using the BnsDatafilePlugin SDK.
 * @author LEaN
 *
 * This file provides a starting point for plugin creators, including example hooks,
 * ImGui panel registration, and plugin metadata definitions.
 */

using namespace PluginGlobalState;
using namespace BnsTables::Dynamic;

static std::unique_ptr<std::thread> g_setupThread;


static void SetupEE() {
	if (g_dataManager == nullptr || g_oFind == nullptr) return;

	bool foundEE1 = false;
	bool foundEE2 = false;

	ForEachRecord<icontexture_Record>(g_dataManager, L"icontexture", [&](icontexture_Record* record, size_t) {
		if (record == nullptr) return true;
		if (foundEE1 && foundEE2) return false; //stop iteration if both found
		if (record->alias && wcscmp(record->alias, L"Costume_65071_JinF_col1") == 0) {
			eeIconTextureId = record->key.key;
			foundEE1 = true;
			return true;
		}
		if (record->alias && wcscmp(record->alias, L"Costume_19169_JinM_col1") == 0) {
			ee2IconTextureId = record->key.key;
			foundEE2 = true;
			return true;
		}
		return true; // continue iteration
		});
}

/**
 * @brief Asynchronous setup function.
 *
 * This function performs any necessary setup operations asynchronously.
 * It sets the `isReady` flag to indicate that the plugin is ready for use.
 * If you need any asynchronous data setup, do it here. This runs in a separate thread and does not block the game.
 */
static void AsyncSetup() {
	{
		std::lock_guard<std::mutex> lock(g_setupMutex);
		SetupEE();
		g_isReady.store(true, std::memory_order_release); //Indicate that setup is complete
	}
	g_setupCv.notify_one();
}

/**
 * @brief Initializes the plugin and registers the ImGui panel.
 *
 * Called by the host application when the plugin is loaded.
 *
 * @param params Initialization parameters provided by the host.
 */
static void __fastcall Init(PluginInitParams* params) {
	g_pluginConfig.Load();

	if (params && params->registerImGuiPanel && params->unregisterImGuiPanel && params->imgui)
	{
		g_imgui = params->imgui;
		g_registerUiPanel = params->registerImGuiPanel;
		g_unregisterUiPanel = params->unregisterImGuiPanel;
		ImGuiPanelDesc desc = { PLUGIN_NAME, ConfigUiPanel, nullptr };
		g_mainPanelHandle = g_registerUiPanel(&desc, false);
	}
	if (params && params->getWorld) {
		g_getWorld = params->getWorld;
	}
	if (params && params->findPatternInMemory) {
		g_findPatternInMemory = params->findPatternInMemory;
	}
	if (params && params->findPatternInAllReadableSections)
	{
		g_findPatternInAllReadableSections = params->findPatternInAllReadableSections;
	}
	if (params && params->dataManager && params->oFind) {
		g_dataManager = params->dataManager;
		g_oFind = params->oFind;

		// Asynchronous setup
		g_setupThread = std::make_unique<std::thread>([]() { AsyncSetup(); });
	}
}

/**
 * @brief Unregisters the ImGui panel and performs cleanup.
 *
 * Called by the host application when the plugin is unloaded.
 */
static void __fastcall Unregister() {
	{
		std::unique_lock<std::mutex> lock(g_setupMutex);
		if (!g_setupCv.wait_for(lock, std::chrono::seconds(5), [] { return g_isReady.load(); })) {
			Logger::LogError("Setup thread did not finish before plugin unload.");
			// skip join, force reset
		}
		else if (g_setupThread && g_setupThread->joinable()) {
			g_setupThread->join();
		}
	}
	g_setupThread.reset();
	if (g_unregisterUiPanel && g_mainPanelHandle != 0) {
		g_unregisterUiPanel(g_mainPanelHandle);
		g_mainPanelHandle = 0;
	}
	g_pluginConfig.Save();
}

/// @brief Table handler array mapping table names to detour functions.
PluginTableHandler handlers[] = {
	{ L"item", &DatafileItemDetour }
};

//static bool healthy = true;
//
//static PluginStatus __fastcall GetPluginStatus() {
//	PluginStatus status;
//	status.success = healthy;
//	status.message = "";
//	return status;
//}

// Plugin metadata and registration macros
DEFINE_PLUGIN_API_VERSION()
DEFINE_PLUGIN_IDENTIFIER(PLUGIN_NAME)
DEFINE_PLUGIN_VERSION(PLUGIN_VERSION)
DEFINE_PLUGIN_INIT(Init, Unregister)
DEFINE_PLUGIN_TABLE_HANDLERS(handlers)
//DEFINE_PLUGIN_STATUS(GetPluginStatus)


//! Example to get a data record
//auto itemRecord = GetRecord<BnsTables::Dynamic::item_Record>(g_dataManager, L"item", 4295917336, g_oFind);

//! Example to iterate data records
//ForEachRecord<BnsTables::Dynamic::item_Record>(g_dataManager, L"item", [](BnsTables::Dynamic::item_Record* record, size_t index) {
//	// Do something with each item record
//	std::wcout << L"Item Name: " << record->alias << std::endl;
//	return true; //Return true to continue the loop. Return false to break the loop.
//	});

//! Log Example: Don't spam the log.
//Logger::Log(std::string(PLUGIN_NAME) + " (v" + std::string(PLUGIN_VERSION) + ") async setup complete.");
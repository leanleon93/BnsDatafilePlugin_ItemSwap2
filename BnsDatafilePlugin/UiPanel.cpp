#include "UiPanel.h"
#include "GlobalState.h"
#include <imgui_plugin_api.h>
#include "pluginversion.h"
#include <string>

using namespace PluginGlobalState;

/**
 * @brief ImGui panel callback for the plugin.
 *
 * This function is called to render the plugin's configuration panel in the UI.
 *
 * @param userData Optional user data pointer (unused in this template).
 */
void ConfigUiPanel(void* userData) {
	auto* imgui = g_imgui;
	if (imgui == nullptr) {
		return;
	}
	if (!g_isReady.load(std::memory_order_acquire)) {
		std::string msg = std::string(PLUGIN_NAME) + " is initializing, please wait...";
		imgui->TextColored(1.0f, 0.5f, 0.0f, 1.0f, msg.c_str());
		return;
	}
	if (imgui->Checkbox((std::string("Enable ") + PLUGIN_NAME).c_str(), &g_pluginConfig.configData.enabled)) {
		g_pluginConfig.Save();
	}
	imgui->Separator();
	imgui->Text("This is a template plugin.");
	imgui->Text("You can use this panel to configure your plugin.");
	imgui->Separator();
	imgui->Text("This plugin does not do anything yet.");
}
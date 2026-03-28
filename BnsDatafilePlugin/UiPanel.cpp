#include "UiPanel.h"
#include "GlobalState.h"
#include <imgui_plugin_api.h>
#include "pluginversion.h"
#include <string>
#include <string.h>
#include <memory>
#include "UiPanels/ItemBrowserUiPanel.hpp"
#include "plugin_config.h"

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

	g_imgui->Separator();
	if (g_imgui->Button("Open Item Browser")) {
		g_itemBrowserOpen = !g_itemBrowserOpen;
	}
	g_imgui->Separator();
	static char newSwapName[128] = "";
	static auto frameHeight = imgui->GetFrameHeight();
	if (g_imgui->CollapsingHeader("Item Swaps")) {
		imgui->Columns(5, "ItemColumns", false);

		imgui->Text("");
		imgui->SetColumnWidth(0, frameHeight * 2);
		imgui->NextColumn();

		imgui->Text("Name");
		imgui->NextColumn();

		imgui->Text("From Id");
		imgui->SetColumnWidth(2, 75.0f);
		imgui->NextColumn();

		imgui->Text("To Id");
		imgui->SetColumnWidth(3, 75.0f);
		imgui->NextColumn();

		imgui->Text("");
		imgui->SetColumnWidth(4, 100.0f);
		imgui->NextColumn();

		imgui->Separator();

		int idx = 0;
		auto& itemSwaps = g_pluginConfig.configData.GetItemSwaps();
		for (auto& swap : itemSwaps) {
			imgui->PushIdInt(idx);

			if (g_imgui->Checkbox("", &swap.enabled)) {
				g_pluginConfig.configData.InvalidateCache();
				g_pluginConfig.Save();
			}
			imgui->NextColumn();

			std::string nameStr = swap.name;
			const size_t maxNameLength = 20; // Maximum length for the name
			if (nameStr.length() > maxNameLength) {
				nameStr = nameStr.substr(0, maxNameLength - 3) + "...";
			}
			imgui->Spacing();
			imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, "%s", nameStr.c_str());
			imgui->NextColumn();

			imgui->Spacing();
			imgui->Text("%d", swap.fromId);
			imgui->NextColumn();

			imgui->Spacing();
			imgui->Text("%d", swap.toId);
			imgui->NextColumn();


			// Actions
			imgui->Spacing();
			if (g_imgui->SmallButton("Edit")) {
				strncpy_s(newSwapName, swap.name.c_str(), sizeof(newSwapName) - 1);
				g_newSwapFrom = swap.fromId;
				g_newSwapTo = swap.toId;
				g_pluginConfig.configData.RemoveItemSwap(swap);
				g_pluginConfig.Save();
				break; // Break out of the loop since the vector has changed
			}
			g_imgui->SameLine(0.0f, 5.0f);
			if (g_imgui->SmallButton("Remove")) {
				g_pluginConfig.configData.RemoveItemSwap(swap);
				g_pluginConfig.Save();
				break; // Break out of the loop since the vector has changed
			}
			imgui->NextColumn();

			imgui->PopId();
			++idx;
		}
		imgui->Columns(1, nullptr, false);
	}
	g_imgui->Spacing();
	g_imgui->Spacing();

	// Add a new swap
	g_imgui->InputText("New Swap Name", newSwapName, sizeof(newSwapName));
	g_imgui->InputInt("From ID", &g_newSwapFrom);
	g_imgui->InputInt("To ID", &g_newSwapTo);
	if (g_imgui->Button("Add")) {
		if (strlen(newSwapName) > 0 && g_newSwapFrom != 0 && g_newSwapTo != 0) {
			PluginConfig::ItemSwap swap;
			swap.name = std::string(newSwapName);
			swap.fromId = g_newSwapFrom;
			swap.toId = g_newSwapTo;
			swap.enabled = true;
			g_pluginConfig.configData.AddItemSwap(swap);
			g_pluginConfig.Save();
			// Clear input
			newSwapName[0] = '\0';
			g_newSwapFrom = 0;
			g_newSwapTo = 0;
		}
	}

	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();
	if (g_imgui->Button("Save Configuration")) {
		g_pluginConfig.Save();
	}
	if (g_imgui->Button("Reload Configuration")) {
		g_pluginConfig.Load();
	}
	ItemBrowserUi(nullptr);
}
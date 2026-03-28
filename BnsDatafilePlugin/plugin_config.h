#pragma once
#include <string>
#include "pluginversion.h"
#include "serialization_macros.hpp"
#include <vector>
#include <unordered_map>
#include <algorithm>

class PluginConfig {
public:
	struct ItemSwap {
		std::string name = "";
		int fromId = 0;
		int toId = 0;
		bool enabled = true;

		bool operator==(const ItemSwap& other) const {
			return name == other.name && fromId == other.fromId && toId == other.toId && enabled == other.enabled;
		}
	};

	struct ConfigData {
		bool enabled = true;
		std::vector<ItemSwap> itemSwaps = {};

	private:
		mutable std::unordered_map<int, int> cachedSwapMap = {}; //not serialized, generated on demand for quick lookups
		mutable bool cacheValid = false; //not serialized, indicates if the cache is valid or needs to be regenerated

		// Invalidate cache when data changes
		void InvalidateCache() {
			cacheValid = false;
		}
	public:

		std::vector<ItemSwap>& GetItemSwaps() {
			return itemSwaps;
		}

		bool IsEnabled() const {
			return enabled;
		}

		std::unordered_map<int, int> GetEnabledSwapMap() const {
			// Return cached map if valid
			if (cacheValid) {
				return cachedSwapMap;
			}

			// Recompute the map
			cachedSwapMap.clear();
			if (enabled) {
				cachedSwapMap.reserve(itemSwaps.size());
				for (const auto& swap : itemSwaps) {
					if (swap.enabled) {
						cachedSwapMap.emplace(swap.fromId, swap.toId);
					}
				}
			}

			// Mark cache as valid
			cacheValid = true;
			return cachedSwapMap;
		}

		void AddItemSwap(const ItemSwap& swap) {
			//check if a swap with the same fromId and toId already exists
			auto it = std::find_if(itemSwaps.begin(), itemSwaps.end(), [&swap](const ItemSwap& existingSwap) {
				return existingSwap.fromId == swap.fromId && existingSwap.toId == swap.toId;
				});
			if (it == itemSwaps.end()) {
				itemSwaps.push_back(swap);
				InvalidateCache();
			}
		}

		void RemoveItemSwap(const ItemSwap& swap) {
			auto it = std::remove_if(itemSwaps.begin(), itemSwaps.end(), [&swap](const ItemSwap& existingSwap) {
				return existingSwap.name == swap.name && existingSwap.fromId == swap.fromId && existingSwap.toId == swap.toId && existingSwap.enabled == swap.enabled;
				});
			if (it != itemSwaps.end()) {
				itemSwaps.erase(it, itemSwaps.end());
				InvalidateCache();
			}
		}

		void SetItemSwaps(const std::vector<ItemSwap>& swaps) {
			itemSwaps = swaps;
			InvalidateCache();
		}

		void SetEnabled(bool isEnabled) {
			enabled = isEnabled;
			InvalidateCache();
		}
	};

	// Properties
	ConfigData configData = {};

	PluginConfig() : filePath("datafilePlugins/" + std::string(PLUGIN_NAME) + "_config.json") {}

	// Set a custom file path
	void SetFilePath(const std::string& path) { filePath = path; }

	// Methods
	void Load();
	void Save() const;

private:
	std::string filePath;
	void Validate();
};

DEFINE_GLZ_META(PluginConfig::ItemSwap,
	GLZ_FIELD(name),
	GLZ_FIELD(fromId),
	GLZ_FIELD(toId),
	GLZ_FIELD(enabled)
)

DEFINE_GLZ_META(PluginConfig::ConfigData,
	GLZ_FIELD(enabled),
	GLZ_FIELD(itemSwaps)
)
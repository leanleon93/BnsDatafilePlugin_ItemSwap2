#pragma once
#include <string>
#include "pluginversion.h"
#include "serialization_macros.hpp"

class PluginConfig {
public:
	struct ConfigData {
		bool enabled = true;
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

DEFINE_GLZ_META(PluginConfig::ConfigData,
	GLZ_FIELD(enabled)
)
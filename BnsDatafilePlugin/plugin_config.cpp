#include "plugin_config.h"
#ifdef PLUGIN_ENABLE_PERSISTENCE
#include "Logger.h"
#endif
void PluginConfig::Load()
{
#ifdef PLUGIN_ENABLE_PERSISTENCE
	if (std::filesystem::exists(filePath)) {
		auto ec = glz::read_file_json(configData, filePath, std::string{});
		auto loaded = !bool(ec);
		if (!loaded) {
			std::ostringstream errorMessage;
			errorMessage << "Failed to load plugin config from " << filePath
				<< ": Error Code " << static_cast<uint32_t>(ec.ec);
			Logger::LogError(errorMessage.str());
			configData = {};
		}
	}
	else {
		configData = {};
	}
#else 
	configData = {};
#endif
	Validate();
}

void PluginConfig::Save() const
{
#ifdef PLUGIN_ENABLE_PERSISTENCE
	// Convert filePath (std::string) to std::filesystem::path
	std::filesystem::path pathObj = filePath;

	// Ensure the directory for the file exists
	auto dir = pathObj.parent_path();
	if (!std::filesystem::exists(dir)) {
		std::error_code ec;
		if (!std::filesystem::create_directories(dir, ec)) {
			Logger::LogError("Failed to create directories for plugin config: " + dir.string() + ". Error: " + ec.message());
			return;
		}
	}

	// Write the configuration to the file
	auto ec = glz::write_file_json < glz::opts{ .prettify = true } > (configData, filePath, std::string{});
	if (ec) {
		std::ostringstream errorMessage;
		errorMessage << "Failed to save plugin config to " << filePath
			<< ": Error Code " << static_cast<uint32_t>(ec.ec);
		Logger::LogError(errorMessage.str());
	}
#endif
}

void PluginConfig::Validate()
{
	// Currently no specific validation rules
}

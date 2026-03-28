# BNS Datafile Plugin Template [![Build All](https://github.com/leanleon93/BnsDatafilePlugin_CmakeTemplate/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/leanleon93/BnsDatafilePlugin_CmakeTemplate/actions/workflows/cmake-single-platform.yml)

A comprehensive template for creating datafile plugins for Blade & Soul using my BnsDatafilePlugin SDK. This template provides a starting point for developers who want to create plugins that can intercept, modify, or extend game data table lookups.

## 🚀 Features

- **Table Interception**: Hook into game data table lookups and modify behavior
- **ImGui Integration**: Built-in UI panels for plugin configuration and debugging
- **Example Implementation**: Complete working example with item table detour
- **Comprehensive Documentation**: Well-documented code with examples

## 📋 Prerequisites

- **Windows 10 SDK** (10.0.19041.0 or later)
- **DatafilePluginSdk** (included in `external/PluginSdk/`)
- **BnsPluginTables** (included in `external/BnsPluginTables/`)

## 🛠️ Building the Plugin

### Build Configurations

| Configuration | Platform | Output | Use Case |
|--------------|----------|---------|----------|
| Debug | x64 | Template.dll | Development & Testing |
| Release | x64 | Template.dll | Production |
| Debug with Persistence | x64 | Template.dll | Development & Testing with Persistence |
| Release with Persistence | x64 | Template.dll | Production with Persistence |

## 🔧 Usage & Customization

### Basic Plugin Structure

The template demonstrates the essential components of a datafile plugin:

```cpp
// 1. Define detour functions for specific tables
static PluginReturnData __fastcall DatafileItemDetour(PluginExecuteParams* params) {
    // Intercept item lookups and optionally modify behavior
    return {};
}

// 2. Create ImGui panels for configuration
static void TemplateUiPanel(void* userData) {
    g_imgui->Text("Plugin configuration goes here");
}

// 3. Register table handlers
PluginTableHandler handlers[] = {
    { L"item", &DatafileItemDetour }
};
```

### Customizing for Your Plugin

1. **Modify Plugin Metadata in CMakeLists.txt**:
   ```cpp
   set(PLUGIN_NAME "Template")
   project ("${PLUGIN_NAME}Plugin" VERSION 1.0.0)
   ```
   **ONLY CHANGE THE PLUGIN_NAME AND VERSION HERE!  
   DO NOT CHANGE THE PROJECT NAME (BnsDatafilePlugin) OR OTHER CMAKE CONFIG UNLESS YOU ARE EXPERIENCED!**

3. **Add Table Handlers**:
   ```cpp
   PluginTableHandler handlers[] = {
       { L"item", &YourItemDetour },
       { L"skill", &YourSkillDetour },
       // Add more table handlers as needed
   };
   ```

4. **Implement Custom Logic**:
   - Modify the detour functions to implement your specific functionality
   - Use `params->oFind()` to perform original lookups
   - Return `PluginReturnData` with a replacement element if needed
   - Use `params->displaySystemChatMessage()` for ingame chat messages

5. **Customize UI Panel**:
   - Modify `ConfigUiPanel()` to add configuration options
   - Use ImGui API through the `g_imgui` pointer or `imgui` alias

### Available Tables

The plugin can intercept lookups for all game tables (see `deps/BnsPluginTables/Generated/include/EU/`)

## 🎯 Examples:

### Item Redirection
The template includes a commented example showing how to redirect one item to another:

```cpp
static PluginReturnData __fastcall DatafileItemDetour(PluginExecuteParams* params) {
    PLUGIN_DETOUR_GUARD_REGION(params);
    unsigned __int64 key = params->key;
    
    // Redirect item key 4295877296 to 4295917336
    if (key == 4295877296) {
        BnsTables::Shared::DrEl* result = params->oFind(params->table, 4295917336);
        return { result };
    }
    
    return {}; // No modification
}
```

### Get any data record
```cpp
auto itemRecord = GetRecord<BnsTables::Dynamic::item_Record>(g_dataManager, L"item", 4295917336, g_oFind);
```

### Iterate over data records in a table
```cpp
ForEachRecord<BnsTables::Dynamic::item_Record>(g_dataManager, L"item", [](BnsTables::Dynamic::item_Record* record, size_t index) {
	// Do something with each item record
	std::wcout << L"Item Name: " << record->alias << std::endl;
	return true; //Return true to continue the loop. Return false to break the loop.
});
```

### Logging to file !Do not spam the log!
The template includes a logging setup into datafilePlugins/logs
```cpp
Logger::Log(std::string(PLUGIN_NAME) + " (v" + std::string(PLUGIN_VERSION) + ") async setup complete.");
Logger::LogError(std::string(PLUGIN_NAME) + " (v" + std::string(PLUGIN_VERSION) + ") async setup failed.");
```

## 🛠️ Conditional Compilation

Conditional compilation allows you to include or exclude parts of the code based on compile-time conditions. For example:

Build with ENABLE_PERSISTENCE = true  
This setting determines whether the plugin stores its plugin_config in a file or not.
```cpp
#ifdef PLUGIN_ENABLE_PERSISTENCE
    // Code for persistence feature
#endif
```

Build with BNSKR = true  
This setting allows you to compile the plugin specifically for the Korean version of the game.
```cpp
#ifdef BNSKR
    // Code specific to KR version
#else 
    // Code for other versions
#endif
```

If you use Visual Studio, you can select the desired build configuration from the toolbar.

## 🔍 Debugging

### Debug Features

- **System Chat Messages**: Use `params->displaySystemChatMessage()` to output debug info ingame
- **ImGui Panels**: Create interactive debug panels
- **Visual Studio Debugging**: Attach to the game process for step-through debugging

### Guard Macros

```cpp
PLUGIN_DETOUR_GUARD_REGION(params);
```

Use this guard macro at the beginning of detour functions for proper version check and error handling.  
This makes the detour not execute if the compiled table version does not match the game version.
## 📚 API Reference

### Core Functions

- `PluginExecuteParams`: Parameters passed to detour functions
- `PluginInitParams`: Initialization parameters
- `PluginReturnData`: Return structure for detour functions
- `PluginTableHandler`: Maps table names to detour functions

### ImGui Integration

Access ImGui functionality through the `g_imgui` pointer or `imgui` alias:

For example:
```cpp
g_imgui->Text("Hello World");
g_imgui->Button("Click Me");
g_imgui->InputText("Input", buffer, sizeof(buffer));
```

## 🚨 Important Notes

- **Plugin API Version**: Make sure to keep your submodules updated for best compatibility
- **Memory Management**: Be careful with memory allocation/deallocation
- **Game Compatibility**: Test thoroughly with target game version
- **Performance**: Minimize processing in detour functions to avoid game lag

## 📄 License

This template is provided as-is for educational and development purposes.

## 🔗 Resources

- [DatafilePluginloader](https://github.com/leanleon93/BnsPlugin_DatafilePluginloader)
- [ImGui Documentation](https://github.com/ocornut/imgui)

---

**Author**: LEaN

#include "ItemBrowserUiPanel.hpp"
#include "../GlobalState.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <Dynamic/item/AAA_item_RecordBase.h>
#include <Dynamic/text/AAA_text_RecordBase.h>
#include <DatafilePluginsdk.h>
#include <algorithm>
#include <sstream>
#include <string.h>

using namespace PluginGlobalState;
using namespace BnsTables::Dynamic;

struct ItemBrowserEntry {
	int id = 0;
	__int16 level = 0;
	std::wstring name = L"";
	signed char category1 = 0;
	signed char category2 = 0;
	short category3 = 0;
};

static std::string DecodeHtmlEntities(const std::string& input) {
	static const std::unordered_map<std::string, char> entityMap = {
		{"&apos;", '\''},
		{"&quot;", '\"'},
		{"&amp;", '&'},
		{"&lt;", '<'},
		{"&gt;", '>'}
		// Add more entities as needed
	};

	std::string result = input;
	for (const auto& [entity, ch] : entityMap) {
		size_t pos = 0;
		while ((pos = result.find(entity, pos)) != std::string::npos) {
			result.replace(pos, entity.length(), 1, ch);
			pos += 1;
		}
	}
	// Optionally, handle numeric entities like &#39; here if needed
	return result;
}

// Converts a std::wstring to a std::string (UTF-8)
inline static std::string WStringToString(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}

// Returns true if all words in 'needle' appear in order in 'haystack'
static bool WordsInOrderMatch(const std::string& haystack, const std::string& needle) {
	std::istringstream needleStream(needle);
	std::string needleWord;
	size_t pos = 0;
	while (needleStream >> needleWord) {
		pos = haystack.find(needleWord, pos);
		if (pos == std::string::npos)
			return false;
		pos += needleWord.length();
	}
	return true;
}

// Copies a UTF-8 std::string to the Windows clipboard
inline static void CopyStringToClipboard(const std::string& str) {
	if (str.empty()) return;

	// Convert UTF-8 std::string to UTF-16 wstring
	int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	if (wlen == 0) return;
	std::wstring wstr(wlen, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], wlen);

	// Open clipboard
	if (!OpenClipboard(nullptr)) return;
	EmptyClipboard();

	// Allocate global memory for the clipboard data
	size_t bytes = (wstr.size() + 1) * sizeof(wchar_t);
	HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, bytes);
	if (!hGlob) {
		CloseClipboard();
		return;
	}

	// Copy the string into the global memory
	void* pGlob = GlobalLock(hGlob);
	if (pGlob) {
		memcpy(pGlob, wstr.c_str(), bytes);
		GlobalUnlock(hGlob);
	}
	// Set clipboard data as Unicode text
	SetClipboardData(CF_UNICODETEXT, hGlob);

	// Close clipboard (do not free hGlob, clipboard owns it now)
	CloseClipboard();
}

static void FillItemCache(std::unordered_map<unsigned __int64, ItemBrowserEntry>& itemCache) {
	if (g_dataManager == nullptr) return;
	auto itemTable = GetTable(g_dataManager, L"item");
	ForEachRecord<item_Record>(itemTable, [&](item_Record* record, size_t) {
		if (record == nullptr) return true;
		ItemBrowserEntry entry;
		entry.id = record->key.id;
		entry.level = record->key.level;
		if (record->name2.Key != 0) {
			auto itemName = GetText<text_Record>(g_dataManager, record->name2.Key, g_oFind);
			if (itemName && itemName->text.ReadableText) {
				entry.name = itemName->text.ReadableText;
			}
			else {
				entry.name = L"(No Name)";
			}
		}
		entry.category1 = record->game_category_1;
		entry.category2 = record->game_category_2;
		entry.category3 = record->game_category_3;
		itemCache[record->key.key] = entry;
		return true; // continue iteration
		});
}

void ItemBrowserUi(void* userData) {
	auto* imgui = g_imgui;
	if (imgui == nullptr || !g_itemBrowserOpen) {
		return;
	}
	static bool firstOpen = true;
	static std::unordered_map<unsigned __int64, ItemBrowserEntry> itemCache;
	static std::vector<const ItemBrowserEntry*> sortedEntries;
	static std::vector<const ItemBrowserEntry*> filteredEntries;
	static bool cacheBuilt = false;
	static char lastSearchBuffer[128] = "";

	// Build the cache and sort entries only once
	if (!cacheBuilt) {
		FillItemCache(itemCache);

		// Sort the entries once
		sortedEntries.clear();
		for (const auto& kv : itemCache) {
			sortedEntries.push_back(&kv.second);
		}
		std::sort(sortedEntries.begin(), sortedEntries.end(), [](const ItemBrowserEntry* a, const ItemBrowserEntry* b) {
			if (a->id != b->id) return a->id < b->id;
			return a->level < b->level;
			});

		// Initialize filteredEntries with all items for the initial render
		filteredEntries = sortedEntries;

		cacheBuilt = true;
	}

	if (firstOpen) {
		g_imgui->SetNextWindowSize(600.0f, 400.0f, 1);
		firstOpen = false;
	}

	g_imgui->Begin("Item Browser (Beta)", &g_itemBrowserOpen, 32);
	static char searchBuffer[128] = "";
	g_imgui->InputText("Search", searchBuffer, sizeof(searchBuffer));
	g_imgui->Separator();

	// Filter items only when the search buffer changes
	if (strcmp(searchBuffer, lastSearchBuffer) != 0) {
		strcpy_s(lastSearchBuffer, searchBuffer);

		std::string searchStr = searchBuffer;
		std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

		filteredEntries.clear();
		if (searchStr.empty()) {
			// If the search buffer is empty, display all items
			filteredEntries = sortedEntries;
		}
		else {
			// Otherwise, filter the items
			for (const auto* entry : sortedEntries) {
				std::string nameStr = WStringToString(entry->name);
				std::string nameStrLower = nameStr;
				std::transform(nameStrLower.begin(), nameStrLower.end(), nameStrLower.begin(), ::tolower);

				// Convert item ID to string for comparison
				std::string idStr = std::to_string(entry->id);

				// Check if the search term matches the name or the ID
				if (WordsInOrderMatch(nameStrLower, searchStr) || idStr.find(searchStr) != std::string::npos) {
					filteredEntries.push_back(entry);
				}
			}
		}
	}

	imgui->Columns(5, "ItemColumns", false);

	imgui->Text("ID");
	imgui->SetColumnWidth(0, 65.0f);
	imgui->NextColumn();

	imgui->Text("Level");
	imgui->SetColumnWidth(1, 55.0f);
	imgui->NextColumn();

	imgui->Text("Name");
	imgui->SetColumnWidth(2, 275.0f);
	imgui->NextColumn();

	imgui->Text("Categories");
	imgui->SetColumnWidth(3, 90.0f);
	imgui->NextColumn();

	imgui->Text("");
	imgui->SetColumnWidth(4, 200.0f);
	imgui->NextColumn();

	imgui->Separator();

	// Iterate only through the filtered entries
	int idx = 0;
	for (const auto* entry : filteredEntries) {
		g_imgui->PushIdInt(idx);
		std::string nameStr = WStringToString(entry->name);

		imgui->Text("%d", entry->id);
		imgui->NextColumn();

		imgui->Text("%d", entry->level);
		imgui->NextColumn();

		imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, "%s", DecodeHtmlEntities(nameStr).c_str());
		imgui->NextColumn();

		imgui->Text("%d/%d/%d", entry->category1, entry->category2, entry->category3);
		imgui->NextColumn();

		if (g_imgui->SmallButton("Copy ID")) {
			CopyStringToClipboard(std::to_string(entry->id));
		}
		g_imgui->SameLine(0, 5.0f);
		if (g_imgui->SmallButton("Set from")) {
			g_newSwapFrom = entry->id;
		}
		g_imgui->SameLine(0, 5.0f);
		if (g_imgui->SmallButton("Set to")) {
			g_newSwapTo = entry->id;
		}
		imgui->NextColumn();
		g_imgui->PopId();
		++idx;
	}
	imgui->Columns(1, nullptr, false);

	g_imgui->End();
}
#pragma once
#include "pluginversion.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <ostream>
#include <string>

namespace Logger {
	static void Log(const std::string& message, const char* level = "INFO") {
		const std::string logFilePath = "datafilePlugins/logs/" + std::string(PLUGIN_NAME) + ".log";
		const std::size_t maxLogFileSize = static_cast<size_t>(1024) * 1024; // 1 MB limit
		const std::size_t truncateSize = static_cast<size_t>(256) * 1024; // Keep the last 256 KB on clear

		// Ensure the directory exists
		std::filesystem::create_directories("datafilePlugins/logs");

		// Check if the log file exceeds the size limit
		if (std::filesystem::exists(logFilePath) && std::filesystem::file_size(logFilePath) > maxLogFileSize) {
			// Open the file in binary mode to read the last `truncateSize` bytes
			std::ifstream logFileIn(logFilePath, std::ios::binary);
			logFileIn.seekg(-static_cast<std::streamoff>(truncateSize), std::ios::end); // Move to the last `truncateSize` bytes

			std::string truncatedContent;
			truncatedContent.resize(truncateSize);
			logFileIn.read(&truncatedContent[0], truncateSize);
			logFileIn.close();

			// Write the truncated content back to the file
			std::ofstream logFileOut(logFilePath, std::ios::trunc | std::ios::binary);
			logFileOut.write(truncatedContent.c_str(), logFileIn.gcount());
			logFileOut.close();
		}

		// Get the current time
		auto now = std::chrono::system_clock::now();
		auto timeT = std::chrono::system_clock::to_time_t(now);
		std::tm localTime;

		// Use localtime_s for thread-safe local time conversion
		if (localtime_s(&localTime, &timeT) != 0) {
			// If localtime_s fails, log an error and return
			std::ofstream logFile(logFilePath, std::ios::app);
			logFile << "[ERROR] Failed to retrieve local time." << std::endl;
			return;
		}

		// Format the timestamp
		std::ostringstream timestamp;
		timestamp << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

		// Write the log entry
		std::ofstream logFile(logFilePath, std::ios::app);
		logFile << "[" << timestamp.str() << "] [" << level << "] " << message << std::endl;
	}

	static void LogError(const std::string& message) {
		Log(message, "ERROR");
	}
}
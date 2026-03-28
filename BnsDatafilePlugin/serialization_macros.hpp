#pragma once
#include "glaze/glaze.hpp"
#include "Windows.h"

// Macro to generate field names and pointers
#define GLZ_FIELD(name) #name, &T::name
#define GLZ_FIELD_ARRAY(name, index) #name, &T::name[index]

// Macro to define serialization metadata
#define DEFINE_GLZ_META(Type, ...) \
namespace glz { \
    template <> \
    struct meta<Type> { \
        using T = Type; \
        static constexpr auto value = object(__VA_ARGS__); \
    }; \
}

static std::string ConvertWCharToString(const wchar_t* wchar) {
	if (wchar == nullptr) return "";
	if (!wchar) return "";

#ifdef _WIN32
	// Windows-specific conversion
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wchar, -1, nullptr, 0, nullptr, nullptr);
	if (bufferSize == 0) {
		throw std::runtime_error("Error converting wide char to UTF-8");
	}

	std::string utf8String(bufferSize, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wchar, -1, &utf8String[0], bufferSize, nullptr, nullptr);

	// Remove the null terminator appended by WideCharToMultiByte
	utf8String.resize(bufferSize - 1);
	return utf8String;
#else
	// POSIX (Linux/macOS) conversion using iconv
	size_t inSize = wcslen(wchar) * sizeof(wchar_t);
	size_t outSize = inSize * 4;  // UTF-8 can use up to 4 bytes per wchar_t

	std::string utf8String(outSize, '\0');
	char* outPtr = &utf8String[0];
	const char* inPtr = reinterpret_cast<const char*>(wchar);

	iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
	if (cd == (iconv_t)-1) {
		throw std::runtime_error("Error opening iconv");
	}

	if (iconv(cd, const_cast<char**>(&inPtr), &inSize, &outPtr, &outSize) == (size_t)-1) {
		iconv_close(cd);
		throw std::runtime_error("Error converting wide char to UTF-8");
	}

	iconv_close(cd);

	// Resize the output string to the actual size used
	utf8String.resize(outPtr - &utf8String[0]);
	return utf8String;
#endif
}

// Define the custom serializer for wchar_t*
namespace glz {
	template <>
	struct meta<wchar_t*> {
		static constexpr auto value = [](const wchar_t* wstr) {
			return ConvertWCharToString(wstr);
			};
	};

	template <>
	struct meta<const wchar_t*> {
		static constexpr auto value = [](const wchar_t* wstr) {
			return ConvertWCharToString(wstr);
			};
	};
}
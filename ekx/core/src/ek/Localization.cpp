#include "Localization.hpp"
#include <unordered_map>
#include <vector>
#include <ek/serialize/streams.hpp>
#include <ek/debug.hpp>

namespace ek {

bool StringCatalog::init(std::vector<uint8_t>&& sourceData) {
    data = std::move(sourceData);
    if (data.empty()) {
        EK_ERROR("mo-file has no data");
        return false;
    }

    input_memory_stream input{data.data(), data.size()};
    const auto magic = input.read<uint32_t>();
    if (magic != 0x950412DE) {
        EK_ERROR("mo-file should be little-endian (or we are running on big-endian?)");
        return false;
    }

    const auto revision = input.read<uint32_t>();
    if (revision != 0) {
        EK_ERROR_F("mo-file invalid revision: %u", revision);
        return false;
    }

    const auto stringsCount = input.read<uint32_t>();

    struct TranslationInfo final {
        uint32_t stringLength = 0;
        uint32_t stringOffset = 0;
        uint32_t translationLength = 0;
        uint32_t translationOffset = 0;
    };
    std::vector<TranslationInfo> translations{stringsCount};

    const auto offsetOriginalStrings = input.read<uint32_t>();
    const auto offsetTranslatedStrings = input.read<uint32_t>();

    if (data.size() < offsetOriginalStrings + 2 * sizeof(std::uint32_t) * stringsCount) {
        EK_ERROR("mo-file: not enough data for original strings");
        return false;
    }

    input.position(offsetOriginalStrings);
    for (uint32_t i = 0; i < stringsCount; ++i) {
        translations[i].stringLength = input.read<uint32_t>();
        translations[i].stringOffset = input.read<uint32_t>();
    }

    if (data.size() < offsetTranslatedStrings + 2 * 4 * stringsCount) {
        EK_ERROR("mo-file: not enough data for translated strings");
        return false;
    }

    input.position(offsetTranslatedStrings);
    for (uint32_t i = 0; i < stringsCount; ++i) {
        translations[i].translationLength = input.read<uint32_t>();
        translations[i].translationOffset = input.read<uint32_t>();
    }

    for (uint32_t i = 0; i < stringsCount; ++i) {
        auto& translationInfo = translations[i];
        if (data.size() < translationInfo.stringOffset + translationInfo.stringLength ||
            data.size() < translationInfo.translationOffset + translationInfo.translationLength) {
            EK_ERROR("mo-file: not enough data for strings repository");
            return false;
        }

        auto str = reinterpret_cast<const char*>(data.data() + translationInfo.stringOffset);
        auto tra = reinterpret_cast<const char*>(data.data() + translationInfo.translationOffset);
        strings[str] = tra;
    }

    return true;
}

bool StringCatalog::has(const char* text) const {
    return strings.find(text) != strings.end();
}

const char* StringCatalog::get(const char* text) const {
    auto it = strings.find(text);
    return it != strings.end() ? it->second : text;
}

const char* Localization::getText(const char* str) const {
    if (languageCatalog) {
        return languageCatalog->get(str);
    }
    return str;
}

bool Localization::setLanguage(const std::string& lang) {
    language = lang;
    auto it = languages.find(lang);
    if (it != languages.end()) {
        languageCatalog = &it->second;
        return true;
    }
    languageCatalog = nullptr;
    return false;
}

const std::string& Localization::getLanguage() const {
    return language;
}

void Localization::load(const std::string& name, std::vector<uint8_t>&& buffer) {
    auto& catalog = languages[name];
    if (catalog.init(std::move(buffer))) {
        languagesList.push_back(name);
    }
}

const std::vector<std::string>& Localization::getAvailableLanguages() const {
    return languagesList;
}

bool Localization::has(const char* text) const {
    return languageCatalog && languageCatalog->has(text);
}

Localization Localization::instance;

}
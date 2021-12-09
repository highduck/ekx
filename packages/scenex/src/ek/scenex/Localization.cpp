#include "Localization.hpp"
#include <ek/serialize/streams.hpp>
#include <ek/log.h>

namespace ek {

bool StringCatalog::init(LocalResource lr) {
    data = lr;
    if (data.buffer == nullptr) {
        EK_ERROR("mo-file has no data");
        return false;
    }

    input_memory_stream input{data.buffer, data.length};
    const auto magic = input.read<uint32_t>();
    if (magic != 0x950412DE) {
        EK_ERROR("mo-file should be little-endian (or we are running on big-endian?)");
        return false;
    }

    const auto revision = input.read<uint32_t>();
    if (revision != 0) {
        EK_ERROR("mo-file invalid revision: %u", revision);
        return false;
    }

    const auto stringsCount = input.read<uint32_t>();

    struct TranslationInfo final {
        uint32_t stringLength = 0;
        uint32_t stringOffset = 0;
        uint32_t translationLength = 0;
        uint32_t translationOffset = 0;
    };
    Array<TranslationInfo> translations{stringsCount};
    translations.resize(stringsCount);

    const auto offsetOriginalStrings = input.read<uint32_t>();
    const auto offsetTranslatedStrings = input.read<uint32_t>();

    if (data.length < offsetOriginalStrings + 2 * sizeof(std::uint32_t) * stringsCount) {
        EK_ERROR("mo-file: not enough data for original strings");
        return false;
    }

    input.position(offsetOriginalStrings);
    for (uint32_t i = 0; i < stringsCount; ++i) {
        translations[i].stringLength = input.read<uint32_t>();
        translations[i].stringOffset = input.read<uint32_t>();
    }

    if (data.length < offsetTranslatedStrings + 2 * 4 * stringsCount) {
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
        if (data.length < translationInfo.stringOffset + translationInfo.stringLength ||
            data.length < translationInfo.translationOffset + translationInfo.translationLength) {
            EK_ERROR("mo-file: not enough data for strings repository");
            return false;
        }

        auto str = reinterpret_cast<const char*>(data.buffer + translationInfo.stringOffset);
        auto tra = reinterpret_cast<const char*>(data.buffer + translationInfo.translationOffset);
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

StringCatalog::~StringCatalog() {
    data.close();
}

const char* Localization::getText(const char* str) const {
    if (languageCatalog) {
        return languageCatalog->get(str);
    }
    return str;
}

bool Localization::setLanguage(const String& lang) {
    language = lang;
    auto it = languages.find(lang);
    if (it != languages.end()) {
        languageCatalog = &it->second;
        return true;
    }
    languageCatalog = nullptr;
    return false;
}

const String& Localization::getLanguage() const {
    return language;
}

void Localization::load(const char* name, LocalResource lr) {
    auto& catalog = languages[name];
    if (catalog.init(lr)) {
        languagesList.emplace_back(name);
    }
    else {
        languages.erase(name);
    }
}

const Array<String>& Localization::getAvailableLanguages() const {
    return languagesList;
}

bool Localization::has(const char* text) const {
    return languageCatalog && languageCatalog->has(text);
}

Localization Localization::instance;

}
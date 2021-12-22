#include "Localization.hpp"
#include <ek/serialize/streams.hpp>
#include <ek/log.h>

namespace ek {

inline int cmp_key(const void* a, const void* b) {
    const char* ka = ((const CStringPair*) a)->k;
    const char* kb = ((const CStringPair*) b)->k;
    const int r = ka != kb ? strcmp(ka, kb) : 0;
    return r;
}

bool StringCatalog::init(ek_local_res lr) {
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
    TranslationInfo* translations = (TranslationInfo*)alloca(sizeof(TranslationInfo) * stringsCount);

    const auto offsetOriginalStrings = input.read<uint32_t>();
    const auto offsetTranslatedStrings = input.read<uint32_t>();

    if (data.length < offsetOriginalStrings + 2 * sizeof(uint32_t) * stringsCount) {
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

    strings = (CStringPair*)malloc(sizeof(CStringPair) * stringsCount);
    strings_num = stringsCount;
    for (uint32_t i = 0; i < stringsCount; ++i) {
        auto& translationInfo = translations[i];
        if (data.length < translationInfo.stringOffset + translationInfo.stringLength ||
            data.length < translationInfo.translationOffset + translationInfo.translationLength) {
            EK_ERROR("mo-file: not enough data for strings repository");
            return false;
        }

        const char* tra = (const char*) (data.buffer + translationInfo.translationOffset);
        strings[i].k = (const char*) (data.buffer + translationInfo.stringOffset);
        strings[i].v = (const char*) (data.buffer + translationInfo.translationOffset);;
    }
    qsort(strings, strings_num, sizeof(CStringPair), cmp_key);
    return true;
}

const char* StringCatalog::get(const char* text) const {
    const CStringPair key{text, nullptr};
    CStringPair* f = (CStringPair*) bsearch(&key, strings, strings_num, sizeof(CStringPair), cmp_key);
    return f ? f->v : text;
}

void StringCatalog::destroy() {
    ek_local_res_close(&data);
    free(strings);
    strings = nullptr;
    strings_num = 0;
}

const char* Localization::getText(const char* str) const {
    if (str && *str && languageCatalog) {
        return languageCatalog->get(str);
    }
    return str;
}

bool Localization::setLanguage(const char* lang) {
    language = lang;
    for (int i = 0; i < languagesList.size(); ++i) {
        if (languagesList[i] == lang) {
            languageCatalog = languages.begin() + i;
            return true;
        }
    }
    languageCatalog = nullptr;
    return false;
}

const String& Localization::getLanguage() const {
    return language;
}

void Localization::load(const char* name, ek_local_res lr) {
    StringCatalog cat{};
    if (cat.init(lr)) {
        languages.push_back(cat);
        languagesList.push_back(name);
    }
    else {
        cat.destroy();
    }
}

const Array<String>& Localization::getAvailableLanguages() const {
    return languagesList;
}

bool Localization::has(const char* text) const {
    return text && *text && languageCatalog && languageCatalog->get(text) != text;
}

Localization Localization::instance;

}
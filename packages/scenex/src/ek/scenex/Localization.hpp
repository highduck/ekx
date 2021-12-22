#pragma once

#include <ek/local_res.h>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>

// Localization module, works with MO Files (binary PO files)
// inspirations:
// - https://github.com/elnormous/ouzel/tree/master/engine/localization
// - https://github.com/j-jorge/libintl-lite
namespace ek {

struct CStringPair {
    const char* k;
    const char* v;
};

class StringCatalog final {
public:
    /// load MO file, and map c-string pointers to identifiers
    /// sourceData will be preserved
    /// specification: https://www.gnu.org/software/gettext/manual/gettext.html#Binaries
    bool init(ek_local_res lr);

    // returns translated c-string or original text if not found
    [[nodiscard]] const char* get(const char* text) const;

    void destroy();

private:
    ek_local_res data{};
    CStringPair* strings = nullptr;
    uint32_t strings_num = 0;
};

class Localization final {
public:
    bool setLanguage(const char* lang);

    [[nodiscard]] const String& getLanguage() const;

    [[nodiscard]] const char* getText(const char* str) const;

    void load(const char* name, ek_local_res lr);

    [[nodiscard]] const Array<String>& getAvailableLanguages() const;

    [[nodiscard]] bool has(const char* text) const;

public:
    static Localization instance;
private:
    Array<String> languagesList;
    Array<StringCatalog> languages;
    StringCatalog* languageCatalog = nullptr;
    String language;
};

}


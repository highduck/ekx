#pragma once

#include <unordered_map>
#include <ek/local_res.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>

// Localization module, works with MO Files (binary PO files)
// inspirations:
// - https://github.com/elnormous/ouzel/tree/master/engine/localization
// - https://github.com/j-jorge/libintl-lite
namespace ek {

class StringCatalog final {
public:
    /// load MO file, and map c-string pointers to identifiers
    /// sourceData will be preserved
    /// specification: https://www.gnu.org/software/gettext/manual/gettext.html#Binaries
    bool init(ek_local_res lr);

    bool has(const char* text) const;

    // returns translated c-string or original text if not found
    [[nodiscard]] const char* get(const char* text) const;

    ~StringCatalog();
private:
    ek_local_res data{};
    std::unordered_map<String, const char*> strings;
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
    std::unordered_map<String, StringCatalog> languages;
    StringCatalog* languageCatalog = nullptr;
    String language;
};

}


#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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
    bool init(std::vector<uint8_t>&& sourceData);

    // returns translated c-string or original text if not found
    [[nodiscard]] const char* getTranslation(const std::string& text) const;

private:
    std::vector<uint8_t> data;
    std::unordered_map<std::string, const char*> strings;
};

class Localization final {
public:
    bool setLanguage(const std::string& lang);

    [[nodiscard]] const std::string& getLanguage() const;

    const char* getText(const char* str) const;

    void load(const std::string& name, std::vector<uint8_t>&& buffer);

public:
    static Localization instance;
private:
    std::unordered_map<std::string, StringCatalog> languages;
    StringCatalog* languageCatalog = nullptr;
    std::string language;
};

}

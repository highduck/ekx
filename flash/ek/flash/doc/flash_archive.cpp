#include "flash_archive.hpp"

#include <pugixml.hpp>
#include <miniz/zip_file.hpp>
#include <fstream>
#include <ek/util/logger.hpp>
#include <sys/stat.h>

namespace ek::flash {

using pugi::xml_document;

xml_document* basic_entry::xml() const {
    if (!xml_doc_) {
        xml_doc_ = new pugi::xml_document();
        auto res = xml_doc_->load(content().c_str());
        if (!res) {
            EK_ERROR << "XML PARSE ERROR: " << res.description();
            delete xml_doc_;
            xml_doc_ = nullptr;
        }
    }
    return xml_doc_;
}

basic_entry::~basic_entry() {
    delete xml_doc_;
    if (root_ == this) {
        for (const auto& it : children_) {
            delete it.second;
        }
    }
}

xfl_entry::xfl_entry(const path_t& path, basic_entry* root)
        : basic_entry{path, root} {

}

xfl_entry::~xfl_entry() = default;

const std::string& xfl_entry::content() const {
    if (contents_.empty()) {
        std::ifstream t(path_.c_str());
        if (t.good()) {
            std::stringstream buffer;
            buffer << t.rdbuf();
            contents_ = buffer.str();
        }
    }
    return contents_;
}

fla_entry::fla_entry(const path_t& zip_file_path)
        : basic_entry{{}, this} {
    zip_file_ = new miniz_cpp::zip_file(zip_file_path.str());
    //zip_file_->printdir();
}

fla_entry::~fla_entry() = default;

const std::string& fla_entry::content() const {
    auto* zipFile = static_cast<fla_entry*>(root_)->zip_file_;
    if (contents_.empty() && zipFile->has_file(path_.str())) {
        contents_ = zipFile->read(path_.str());
    }
    return contents_;
}

bool is_dir(const path_t& path) {
    struct stat sb{};
    return stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
}

bool is_file(const path_t& path) {
    struct stat sb{};
    return stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode);
}

std::unique_ptr<basic_entry> load_flash_archive(const path_t& path) {
    using namespace ek::flash;

    if (is_file(path)) {
        const auto ext = path.ext();
        // dir/FILE/FILE.xfl
        if (ext == "xfl") {
            auto dir = path.dir();
            if (is_dir(dir)) {
                return std::make_unique<xfl_entry>(dir);
            } else {
                EK_ERROR("Import Flash: loading %s XFL file, but %s is not a dir", path.c_str(), dir.c_str());
            }
        } else if (ext == "fla") {
            return std::make_unique<fla_entry>(path);
        } else {
            EK_ERROR << "Import Flash: file is not xfl or fla: " << path;
        }
    }

    // dir/FILE.fla
    const auto fla_file = path + ".fla";
    if (is_file(fla_file)) {
        return std::make_unique<fla_entry>(fla_file);
    } else if (is_dir(path)) {
        if (is_file(path / path.basename() + ".xfl")) {
            return std::make_unique<xfl_entry>(path);
        } else {
            EK_WARN << "Import Flash: given dir doesn't contain .xfl file: " << path;
        }
    }

    EK_ERROR << "Import Flash: file not found: " << path;

    return nullptr;
}

}
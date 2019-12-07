#include "flash_archive.h"

#include <pugixml.hpp>
#include <miniz/zip_file.hpp>
#include <fstream>
#include <ek/util/logger.hpp>

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

}
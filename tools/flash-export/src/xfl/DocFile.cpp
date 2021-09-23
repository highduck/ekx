#include "Doc.hpp"

#include <pugixml.hpp>
#include <ek/debug.hpp>
#include <sys/stat.h>
#include <miniz.h>
#include <unordered_map>

using pugi::xml_document;

namespace ek::xfl {

File::~File() = default;

class FileNode : public File {
public:

    FileNode(path_t path, FileNode* root) :
            path_{std::move(path)},
            root_{root ? root : this} {

    }

    ~FileNode() override {
        delete xml_doc_;
        if (root_ == this) {
            for (const auto& it: children_) {
                delete it.second;
            }
        }
    }

    pugi::xml_document* xml() const override {
        if (!xml_doc_) {
            xml_doc_ = new pugi::xml_document();
            auto res = xml_doc_->load(this->content().c_str());
            if (!res) {
                EK_ERROR_F("XML PARSE ERROR: %s", res.description());
                delete xml_doc_;
                xml_doc_ = nullptr;
            }
        }
        return xml_doc_;
    }

    const path_t& path() const {
        return path_;
    }

    const File* open(const path_t& relPath) const override {
        auto childPath = path_ / relPath;
        auto& children = root_->children_;
        const auto& key = childPath.str();
        auto it = children.find(key);
        if (it == children.end()) {
            children[key] = create(childPath, root_);
        }
        return children[key];
    }

protected:
    virtual FileNode* create(const path_t& path, FileNode* root) const = 0;

    path_t path_;
    FileNode* root_ = nullptr;
    std::unordered_map<std::string, FileNode*> children_;

    mutable pugi::xml_document* xml_doc_ = nullptr;
    mutable std::string contents_;
};

class XFLNode final : public FileNode {
public:
    XFLNode(const path_t& path, FileNode* root) :
            FileNode{path, root} {
    }

    ~XFLNode() override = default;

    const std::string& content() const override {
        if (contents_.empty()) {
            auto* stream = fopen(path_.c_str(), "r");
            if (stream) {
                fseek(stream, 0, SEEK_END);
                contents_.resize(static_cast<size_t>(ftell(stream)));
                fseek(stream, 0, SEEK_SET);

                fread(contents_.data(), contents_.size(), 1u, stream);

                if (ferror(stream) != 0) {
                    contents_.resize(0);
                    contents_.shrink_to_fit();
                }

                fclose(stream);
            } else {
                EK_ERROR_F("Error read XFL node: %s", path_.c_str());
            }
        }
        return contents_;
    }

protected:

    FileNode* create(const path_t& path, FileNode* root) const override {
        return new XFLNode(path, root);
    }
};


class FLANode final : public FileNode {
public:

    explicit FLANode(const path_t& zip_file_path) : FileNode{{}, this} {
        zip_ = new mz_zip_archive;
        memset(zip_, 0, sizeof(mz_zip_archive));

        // MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY
        auto status = mz_zip_reader_init_file(zip_, zip_file_path.c_str(), 0);
        if (!status) {
            EK_WARN("Error reading FLA zip archive");
        }
    }

    FLANode(const path_t& path, FileNode* root) :
            FileNode{path, root} {
    }

    ~FLANode() override {
        delete zip_;
    }

    const std::string& content() const override {
        auto* zip = static_cast<FLANode*>(root_)->zip_;
        if (contents_.empty()) {
            std::size_t size;
            char* data = (char*) mz_zip_reader_extract_file_to_heap(zip, path_.c_str(), &size, 0);
            if (data != nullptr) {
                contents_.assign(data, data + size);
                mz_free(data);
            }
        }
        return contents_;
    }

protected:

    FileNode* create(const path_t& path, FileNode* root) const override {
        return new FLANode(path, root);
    }

private:
    mz_zip_archive* zip_ = nullptr;
};

bool is_dir(const path_t& path) {
    struct stat sb{};
    return stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
}

bool is_file(const path_t& path) {
    struct stat sb{};
    return stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode);
}

std::unique_ptr<File> File::load(const path_t& path) {
    if (is_file(path)) {
        const auto ext = path.ext();
        // dir/FILE/FILE.xfl
        if (ext == ".xfl") {
            auto dir = path.dir();
            if (is_dir(dir)) {
                return std::make_unique<XFLNode>(dir, nullptr);
            } else {
                EK_ERROR_F("Import Flash: loading %s XFL file, but %s is not a dir", path.c_str(), dir.c_str());
            }
        } else if (ext == ".fla" || ext == ".zip") {
            return std::make_unique<FLANode>(path);
        } else {
            EK_ERROR_F("Import Flash: file is not xfl or fla: %s", path.c_str());
        }
    }

    // dir/FILE.fla
    const auto fla_file = path + ".fla";
    if (is_file(fla_file)) {
        return std::make_unique<FLANode>(fla_file);
    } else if (is_dir(path)) {
        if (is_file(path / path.basename() + ".xfl")) {
            return std::make_unique<XFLNode>(path, nullptr);
        } else {
            EK_WARN_F("Import Flash: given dir doesn't contain .xfl file: %s", path.c_str());
        }
    }

    EK_ERROR_F("Import Flash: file not found: %s", path.c_str());
    return nullptr;
}

}
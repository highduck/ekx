#include "Doc.hpp"

#include <pugixml.hpp>
#include <ek_log.h>
#include <ek_assert.h>
#include <sys/stat.h>
#include <miniz.h>
#include <unordered_map>
#include <utility>
#include <stb/stb_sprintf.h>

using pugi::xml_document;

namespace ek::xfl {

File::~File() = default;

class FileNode : public File {
public:

    FileNode(const char* path, FileNode* root) :
            path_{path},
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
                EK_ERROR("XML PARSE ERROR: %s", res.description());
                delete xml_doc_;
                xml_doc_ = nullptr;
            }
        }
        return xml_doc_;
    }

    const std::string& path() const {
        return path_;
    }

    const File* open(const char* relPath) const override {
        String childPath{path_.c_str()};
        Path::appendJoin(childPath, relPath);
        // TODO: remmmmove std::string everywhere...
        std::string childPath_{childPath.c_str()};
        auto& children = root_->children_;
        auto it = children.find(childPath_);
        if (it == children.end()) {
            children[childPath_] = create(childPath_.c_str(), root_);
        }
        return children[childPath_];
    }

protected:
    virtual FileNode* create(const char* path, FileNode* root) const = 0;

    std::string path_;
    FileNode* root_ = nullptr;
    std::unordered_map<std::string, FileNode*> children_;

    mutable pugi::xml_document* xml_doc_ = nullptr;
    mutable std::string contents_;
};

class XFLNode final : public FileNode {
public:
    XFLNode(const char* path, FileNode* root) :
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
                EK_ERROR("Error read XFL node: %s", path_.c_str());
            }
        }
        return contents_;
    }

protected:

    FileNode* create(const char* path, FileNode* root) const override {
        return new XFLNode(path, root);
    }
};


class FLANode final : public FileNode {
public:

    explicit FLANode(const char* zip_file_path) :
            FileNode{"", this} {
        zip_ = (mz_zip_archive*) malloc(sizeof(mz_zip_archive));
        memset(zip_, 0, sizeof(mz_zip_archive));

        // MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY
        auto status = mz_zip_reader_init_file(zip_, zip_file_path, 0);
        if (!status) {
            EK_WARN("Error reading FLA zip archive: %s", zip_file_path);
        }
    }

    FLANode(const char* path, FileNode* root) :
            FileNode{path, root} {
    }

    ~FLANode() override {
        free(zip_);
    }

    const std::string& content() const override {
        auto* zip = static_cast<FLANode*>(root_)->zip_;
        if (contents_.empty()) {
            size_t size = 0;
            char* data = (char*) mz_zip_reader_extract_file_to_heap(zip, path_.c_str(), &size, 0);
            if (data != nullptr) {
                contents_.assign(data, data + size);
                mz_free(data);
            }
        }
        return contents_;
    }

protected:

    FileNode* create(const char* path, FileNode* root) const override {
        return new FLANode(path, root);
    }

private:
    mz_zip_archive* zip_ = nullptr;
};

bool is_dir(const char* path) {
    struct stat sb{};
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

bool is_file(const char* path) {
    struct stat sb{};
    return stat(path, &sb) == 0 && S_ISREG(sb.st_mode);
}

std::unique_ptr<File> File::load(const char* path) {
    char tmp[1024] = "";

    if (is_file(path)) {
        const char* ext = ek_path_ext(path);
        // dir/FILE/FILE.xfl
        if (strncmp(ext, "xfl", 3) == 0) {
            ek_path_dirname(tmp, 1024, path);
            if (is_dir(tmp)) {
                return std::make_unique<XFLNode>(tmp, nullptr);
            } else {
                EK_ERROR("Import Flash: loading %s XFL file, but %s is not a dir", path, tmp);
            }
        } else if (strncmp(ext, "fla", 3) == 0 || strncmp(ext, "zip", 3) == 0) {
            return std::make_unique<FLANode>(path);
        } else {
            EK_ERROR("Import Flash: file is not xfl or fla: %s", path);
        }
    }

    // dir/FILE.fla
    stbsp_snprintf(tmp, 1024, "%s.fla", path);
    if (is_file(tmp)) {
        return std::make_unique<FLANode>(tmp);
    } else if (is_dir(path)) {
        stbsp_snprintf(tmp, 1024, "%s/%s.xfl", path, ek_path_name(path));
        if (is_file(tmp)) {
            return std::make_unique<XFLNode>(path, nullptr);
        } else {
            EK_WARN("Import Flash: given dir doesn't contain .xfl file: %s", path);
        }
    }

    EK_ERROR("Import Flash: file not found: %s", path);
    return nullptr;
}

}
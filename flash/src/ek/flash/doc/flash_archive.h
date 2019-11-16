#pragma once

#include <utility>
#include <string>
#include <unordered_map>
#include <ek/fs/path.hpp>

namespace miniz_cpp {
class zip_file;
}

namespace pugi {
class xml_document;
}

namespace ek::flash {

class basic_entry {
public:

    basic_entry(path_t path, basic_entry* root)
            : path_{std::move(path)},
              root_{root ? root : this} {

    }

    virtual ~basic_entry();

    pugi::xml_document* xml() const;

    const path_t& path() const {
        return path_;
    }

    virtual const std::string& content() const = 0;

    const basic_entry* open(const path_t& rpath) const {
        auto child_path = path_ / rpath;

        basic_entry* entry = nullptr;
        auto it = root_->children_.find(child_path.str());
        if (it == root_->children_.end()) {
            entry = create(child_path, root_);
            root_->children_[child_path.str()] = entry;
        } else {
            entry = it->second;
        }
        return entry;
    }

protected:

    virtual basic_entry* create(const path_t& path, basic_entry* root) const = 0;

    path_t path_;
    basic_entry* root_ = nullptr;
    std::unordered_map<std::string, basic_entry*> children_;

    mutable pugi::xml_document* xml_doc_ = nullptr;
    mutable std::string contents_;
};

class xfl_entry final : public basic_entry {
public:

    explicit xfl_entry(const path_t& path, basic_entry* root = nullptr);

    ~xfl_entry() override;

    const std::string& content() const override;

protected:

    basic_entry* create(const path_t& path, basic_entry* root) const override {
        return new xfl_entry(path, root);
    }
};


class fla_entry final : public basic_entry {
public:

    explicit fla_entry(const path_t& zip_file_path);

    fla_entry(const path_t& path, basic_entry* root)
            : basic_entry{path, root} {
    }

    ~fla_entry() override;

    const std::string& content() const override;

protected:

    basic_entry* create(const path_t& path, basic_entry* root) const override {
        return new fla_entry(path, root);
    }

private:
    miniz_cpp::zip_file* zip_file_ = nullptr;
};

}



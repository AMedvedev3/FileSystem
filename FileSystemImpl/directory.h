#pragma once
#include "common.h"
#include "file_description.h"
#include "file.h"

using TDirectoryPtr = bip::offset_ptr<Directory>;
using TFileSet = shared_map<shared_string, File>;

class Directory
{
public:

    Directory(const char* name, TDirectoryPtr parent, FileSystem& fs);

    virtual ~Directory()
    {
        std::cout << "Directory destructor" << std::endl;
    }

    const char* get_name() const { return description->name; } 

    std::string get_path() const;

    // get copy of description that describes this directory
    FileDescription get_description();

    TDirectoryPtr find_sub_directory(const shared_string& name) const;

    bip::offset_ptr<File> find_file(const shared_string& name);
    
    void add_file(const shared_string& name, File& file);
    
    void remove_file(const shared_string& name);
    
    void rename(const char* new_name);

    TDirectoryPtr add_directory(FileSystem& fs, const char* name);

    void remove_directory(const shared_string& name);
    bool empty();

    void clean(FileSystem& fs);
    // TODO: add getter directories list - return range of files ?? accessible from FS only

    // Factory for construct Directory object in managed_mapped_file
    static TDirectoryPtr construct(const char* name, TDirectoryPtr parent,
        FileSystem& fs, int l = 0);

    
    // Find directory by their full path, starting from root directory
    // note: the path is a directory path, without file
    static TDirectoryPtr find_directory(FileSystem& fs, TDirectoryPtr& root, const char* path);

public:
    bip::offset_ptr<FileDescription> description;
    bip::offset_ptr<Directory> parent;
    bip::offset_ptr<TDirectorySet> sub_directories;
    bip::offset_ptr<TFileSet> files;
};

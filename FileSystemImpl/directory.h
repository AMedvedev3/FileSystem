#pragma once
/**
@file
Directory class definition for FileSystem
@author Alexey Medvedev
*/

#include "common.h"
#include "file_description.h"
#include "file.h"

using TDirectoryPtr = bip::offset_ptr<Directory>;
using TFileSet = shared_map<shared_string, File>;

// This class represents the directory structure of FileSystem
// It contains subfoders, files and references parent folder
// Referenced Description defines information about directory instance
class Directory
{
public:

    Directory(const char* name, TDirectoryPtr parent, FileSystem& fs);

    // Returns directory name
    const char* get_name() const { return description->name; } 
    
    // Returns current full path to this directory
    std::string get_path() const;

    // Get copy of description that describes this directory
    FileDescription get_description();

    // Find subdirectory of this directory. Non recursive search.
    TDirectoryPtr find_sub_directory(const shared_string& name) const;

    // Find file in this directory. Non recursive search.
    bip::offset_ptr<File> find_file(const shared_string& name);
    
    // Add given file to this directory
    void add_file(const shared_string& name, File& file);
    
    // Remove file that this directory contains
    void remove_file(const shared_string& name);
    
    // Rename directory
    void rename(const char* new_name);

    // Add subdirectory to this directory. 
    // constructs new instance of directory
    TDirectoryPtr add_directory(FileSystem& fs, const char* name);
    
    // Remove directory and it's structures
    // Note: only empty directory can be removed
    // it throws exception if the direcory contains files or subfolders
    void remove_directory(const shared_string& name);

    // Check that directory is empty
    // returns true if directory doesn't contain files and sub folders
    bool empty();

    // function that destroys structures of directory
    void clean(FileSystem& fs);

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

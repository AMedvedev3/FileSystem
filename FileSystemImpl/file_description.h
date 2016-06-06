#pragma once
/**
@file
FileDescription class definition for FileSystem
@author Alexey Medvedev
*/
#include "common.h"

enum DescriptionType { DirectoryType = 0, FileType = 1 };

// The File Descriptions contains file (or directory) meta data
// It keeps the file name, file size, creation date and changed data.
// The File and his FileDescription identified by fileID field.
// Type shows is that description of file or directory
// Note: the dates and attributes of file are not implemented yet.
// The path reserved for future usage.
// TODO: make functionality for create and changed time
// TODO: make file attribute functionality
struct FileDescription
{
    FileDescription(const char* name, const char* path,
        TFileID id, uintmax_t size)
        :fileID(id), size(size)
    {
        set_name(name);
        set_path(path);
    }

    void set_name(const char* new_name)
    {
        if (new_name)
            strcpy_s(this->name, new_name);
    }

    void set_path(const char* new_path)
    {
        if (new_path)
            strcpy_s(this->path, new_path);
    }


    TFileID fileID;
    char name[256];
    char path[256];
    uintmax_t size;
    std::time_t create_time;
    std::time_t change_time;
    DescriptionType type;
    // bitflag attribs

};

using TFileDescriptionPtr = bip::offset_ptr<FileDescription>;

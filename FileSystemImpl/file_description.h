#pragma once
#include "common.h"

enum DescriptionType { DirectoryType = 0, FileType = 1 };

// TODO: make functionality for create and changed time
// TODO: make file attribute functionality
// TODO: try to rework path and name to shared string
// TODO: add flag empty to directories type
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

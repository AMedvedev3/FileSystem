#pragma once
#include "common.h"
#include "file_system.h"

shared_string mkstr(const char* src, const FileSystem& fs);

shared_string mkstr(const char* src, TStoragePtr& seg);

class ParsedPath
{
public:
    vector<string> directories;
    string fileName;

    // construct path string without filename
    std::string get_path();
    // construct path string including filename
    std::string get_full_path();

    static ParsedPath parse_dir_path(const char* path);
    static ParsedPath parse_file_path(const char* path);
};



bool is_valid_path_name(const char* path);

bool is_valid_file_name(const char* name);





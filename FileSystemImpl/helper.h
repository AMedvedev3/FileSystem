#pragma once
/**
@file
This module contains helper functions and utility classes for FileSystem
@author Alexey Medvedev
*/
#include "common.h"
#include "file_system.h"

// Factory constructs the string in the FileSystem storage
shared_string mkstr(const char* src, const FileSystem& fs);

// Factory constructs the string in the FileSystem storage
shared_string mkstr(const char* src, TStoragePtr& seg);

// File or directory path lexicography parser
class ParsedPath
{
public:
    vector<string> directories;
    string fileName;

    // construct path string without filename
    std::string get_path();
    // construct path string including filename
    std::string get_full_path();

    // parse full path to directory
    static ParsedPath parse_dir_path(const char* path);
    // parse full path to file
    static ParsedPath parse_file_path(const char* path);
};


// lexicography validation of directory path string
bool is_valid_path_name(const char* path);

// lexicography validation of file name
bool is_valid_file_name(const char* name);





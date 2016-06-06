/**
@file
This module contains helper functions and utility classes for FileSystem
@author Alexey Medvedev
*/
#include "helper.h"
#include <functional>
#include <numeric>

shared_string mkstr(const char* src, const FileSystem& fs)
{
    return shared_string(src, fs.m_seg->get_segment_manager());
}

shared_string mkstr(const char* src, TStoragePtr& seg)
{
    return shared_string(src, seg->get_segment_manager());
}


ParsedPath ParsedPath::parse_dir_path(const char* path)
{
    ParsedPath res;
    if (path == NULL)
        return res;

    // split the path to directories
    split(res.directories, path, is_any_of("\\/"), token_compress_on);

    // sanity: remove empty strings
    res.directories.erase(
        remove_if(res.directories.begin(), res.directories.end(),
            [](string& s) { return s.empty(); }),
        res.directories.end());

    return res;
}


ParsedPath ParsedPath::parse_file_path(const char* path)
{
    ParsedPath res = ParsedPath::parse_dir_path(path);
    
    // check that file name is available
    if (!res.directories.empty())
    {
        string s = res.directories.back();

        res.fileName = s;
        res.directories.pop_back();
    }
    return res;
}


std::string ParsedPath::get_path()
{
    std::string res;
    if (!directories.empty())
    {

        res = std::accumulate(directories.begin(),
            directories.end(), std::string{},
            [](std::string& r, std::string& s)
        {
            return r.append(s).append("/");
        });
    }

    return res;
}

std::string ParsedPath::get_full_path()
{
    std::string path = get_path();
    if (!path.empty())
        path.append(this->fileName);
    return path;
}

bool is_valid_path_name(const char* path)
{
    if (path == NULL)
        return false;

    string s(path);
    if (s.empty())
        return false;

    // no more then 255 symbols allowed in name
    if (s.length() > 255)
        return false;

    // name should not be started and ended by ' '
    if (s.find_first_not_of(' ') != 0)
        return false;

    if (s.find_last_not_of(' ') + 1 != s.length())
        return false;

    // check for special symbols see 
    // like in Windows https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365247(v=vs.85).aspx#naming_conventions
    // name should not include first 31 chars from ascii table
    auto it = find_if(s.begin(), s.end(), [](char c)
    { return c < 32; });
    if (it != s.end())
        return false;

    // also it should not contain next symbols 
    if (s.find_first_of("<>:?\"|*") != string::npos)
        return false;

    return true;
}

bool is_valid_file_name(const char* name)
{
    // name should not be empty or null
    if (name == NULL)
        return false;

    string s(name);
    if (s.empty())
        return false;

    // no more then 255 symbols allowed in name
    if (s.length() > 255)
        return false;

    // name should not be started and ended by ' '
    if (s.find_first_not_of(' ') != 0)
        return false;

    if (s.find_last_not_of(' ') + 1 != s.length())
        return false;

    // name should contain '.', but it should not be leading or ending char
    size_t pos = s.find('.');
    if (pos == string::npos || pos == 0 || pos + 1 == s.length())
        return false;

    // check for special symbols see 
    // like in Windows https://msdn.microsoft.com/en-gb/library/windows/desktop/aa365247(v=vs.85).aspx#naming_conventions
    // name should not include first 31 chars from ascii table
    auto it = find_if(s.begin(), s.end(), [](char c) 
        { return c < 32; });
    if (it != s.end())
        return false;

    // also it should not contain next symbols 
    if (s.find_first_of("<>:?/\\\"|*") != string::npos)
        return false;
 
    return true;
}


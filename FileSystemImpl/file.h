#pragma once
/**
@file
File class definition for FileSystem
@author Alexey Medvedev
*/
#include "common.h"
#include "file_description.h"

class Directory;
class FileSystem;
// File class represents files in FileSystem
// The File is the  file dispatch object - it just reference between 
// directory, file data and file description.
// It lives inside of Directory
class File
{
public:
    // TODO: the construct() function  of managed_mapped file requires it to be public :(
    // maybe declare managed_mapped_file as a friend?
    File(TFileID id, const char* name, TDirectoryPtr& dir, FileSystem& fs);

    // Read file data
    // read 'cb' bytes into 'buffer' starting from position 'offset' in file
    // returns last position in file after read and ref to count of readed bytes
    // It can return less data then requested if it reach end of file.
    // in that case the 'readed' contains number od actually readed bytes
    size_t read(char* buffer, size_t cb, size_t offset, size_t* readed);

    // Inner file write function
    // buffer - buffer to write
    // cb - size of buffer, number of bytes to write
    // offset - start position in file where to start writing new data
    // written - return value - returns number of really written bytes to file
    // fs - reference to fs for inner purposes
    // l - reentrant counter used for exception handling. Don't set it explicitely, always use default value  
    // returns last position in file after a write finished and ref to count of written bytes
    size_t write(const char* buffer, size_t cb, size_t offset, size_t* written, FileSystem& fs, int l=0);

    // TODO: implement
    void move_to(TDirectoryPtr& directory) {}

    // TODO: implement
    void copy_to(TDirectoryPtr& directory) {}

    // TODO: implement
    void truncate(size_t new_size) {}

    // rename file
    void rename(const char* new_name);

    // get current file size
    size_t get_size();

    // get file name
    std::string get_name();

    // get full path to file. starting from root directory
    std::string get_path();

    // Get reference to directory that contains this file
    TDirectoryPtr get_directory();

    // Get file id in segment db
    TFileID get_id();

    // Get copy of file description
    FileDescription get_description();

    // clean inner file structures. used for delete the file
    void clean(FileSystem& fs);
        
    // Factory that constructs the file
    static bip::offset_ptr<File> construct(const char* name, TFileID id, 
        TDirectoryPtr& dir, FileSystem& fs, int l=0);

protected:

    TFileDescriptionPtr description;
    TDirectoryPtr directory;
    TFileDataPtr file_data;
    TFileID id;
};

using TFilePtr = bip::offset_ptr<File>;
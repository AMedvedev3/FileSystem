#pragma once
#include "common.h"
#include "file_description.h"

class Directory;
class FileSystem;
// The File is the  file dispatch object.
// It lives inside of Directory
class File
{
public:
    // TODO: the construct() function  of managed_mapped file requires it to be public :(
    // maybe declare managed_mapped_file as a friend?
    File(TFileID id, const char* name, TDirectoryPtr& dir, FileSystem& fs);

    // returns last position in file after read and ref to count of readed bytes
    size_t read(char* buffer, size_t cb, size_t offset, size_t* readed);

    // Inner file write function
    // @buffer - buffer to write
    // @cb - size of buffer, number of bytes to write
    // @offset - start position in file where to start writing new data
    // @written - return value - returns number of really written bytes to file
    // @fs - reference to fs for inner purposes
    // @l - reentrant counter used for exception handling. Don't set it explicitely, always use default value  
    // returns last position in file after a write finished and ref to count of written bytes
    size_t write(const char* buffer, size_t cb, size_t offset, size_t* written, FileSystem& fs, int l=0);

    void move_to(TDirectoryPtr& directory) {}

    void copy_to(TDirectoryPtr& directory) {}

    void truncate(size_t new_size) {}

    void rename(const char* new_name);

    size_t get_size();

    std::string get_name();

    std::string get_path();

    TDirectoryPtr get_directory();

    TFileID get_id();

    FileDescription get_description();

    void clean(FileSystem& fs);
        
    static bip::offset_ptr<File> construct(const char* name, TFileID id, 
        TDirectoryPtr& dir, FileSystem& fs, int l=0);

protected:

    TFileDescriptionPtr description;
    TDirectoryPtr directory;
    TFileDataPtr file_data;
    TFileID id;
};

using TFilePtr = bip::offset_ptr<File>;
#pragma once
/**
@file
Root class definition for FileSystem
@author Alexey Medvedev
*/
#include "common.h"
#include "fileid_manager.h"
#include "file.h"

// Roles of File System Manager: - 
// create fs, remove fs, buckup fs, open fs,
// grow file storage.

// Roles of Directory Service - to find file, 
// to enumerate directories and files, navigation by directories,
// to control logical place integrity of directories and files,
// to move file, to remove file.? (file role?)

// Roles of File - open file, write file, 
// under question operation - create file, remove file, move file.

// Roles of FileDescription - identify file by name, by full path, 
// create, store and update metadata information about file

// File system objects grouped to different segments. 
// That allows to decrease number of page loads when we works with same objects
// for example: file descriptions grouped into segment allows to decrease cost of 
// memory page loads when we traverse by files and directories
// Also the file data placed into separate segment that allows to decrease 
// a fragmentation of directory and 
// in the future it have sense to make different segments for different size file data -
// little files segment, big file segments (although it have sense to place large files into separate storage)

// Main class of FileSystem
// Represents Storage functionality and implementation of FileSystem public functions that can be used by clients
// This makes the File System storage in './demo.bin' that it creates in current folder
// TODO: add validation of format, existence of mapped file, consistency of storage.
// TODO: incapsulate hidden members  
class FileSystem
{
public:

    FileSystem() :
        m_StorageName("./demo1.bin"),
        m_files_data(NULL)
    {
        init_file_system();
    };

    FileSystem(char* storageName) :
        m_StorageName(storageName),
        m_files_data(NULL)
    {
        init_file_system();
    }

    /* --- Top level functions --*/

    // Removes Storage. Deletes storage file
    void remove_storage();

    // Creates new file. 
    //'name' is the full path to creating file (including new file name)
    // returns reference to created file
    // It validates path, parent directory.
    // Throws execptions in case of invalid name 
    TFilePtr create_file(const char* name);

    // Creates new directory.
    // 'name' is the full path to creating directory (including new directory name)
    // returns refernce to created directory
    // It validates path, parent directory.
    // Throws execptions in case of invalid name 
    TDirectoryPtr create_directory(const char* path);

    // Writes 'buffer' to existing file 'name'
    // 'name' - full path to file 
    // 'cb' - count of bytes to write
    // 'offset - start position in file to write data
    // 'written' - how much bytes actually written to file 
    //  returns last position in file after write, can be used to chain writes
    size_t write_file(const char* name, const char* buffer, size_t cb, size_t offset, size_t* written);

    // Read into 'buffer' from existing file with 'name'
    // 'name' - full path to file
    // 'cb' - size of buffer to read
    // 'offset - start position in file to read data
    // 'readed' - how much bytes actually readed from file 
    //  returns last position in file after read, can be used to chain reads
    size_t read_file(const char* name, char* buffer, size_t cb, size_t offset, size_t* readed);

    // Remove file 
    // 'name' is the full path to removing file
    // It validates path, parent directory.
    // Throws execptions in case of invalid name 
    void remove_file(const char* file);

    // Remove directory 
    // 'name' is the full path to removing directory
    // It validates path, parent directory.
    // Throws execptions in case of invalid name
    void remove_directory(const char* dir_name);

    // Returns copy of file description
    // name is the full path to file
    FileDescription get_file_desc(const char* name);

    // Returns array of directories and files that given directory contains
    vector<FileDescription> get_directory_list(const char* name);

    /* ---  Inner functions --- */

    // grow the File system storage
    // It closes the opened storage, increase size by 100Mb and open file system again
    void grow_storage();

    // inner function to remove file description
    void remove_file_desc(TFileID id);

    //inner function to add file description
    bip::offset_ptr<FileDescription> add_file_desc(const char* name, const char* path, size_t size);

    // Finds file by full path and directory that contains this file
    // throws exception if doesn't find them
    // used by top level functions
    void find_file_and_dir(const char* name, TDirectoryPtr* out_dir, TFilePtr* out_file);

    // Find directory by full path 
    // throws exception if the file doesn't exists or file name is invalid 
    TDirectoryPtr find_directory(const char* name);

    //protected:
    const char* m_StorageName;

    // TODO: enclose this set of datas into recreating wrapper object
    // TODO: guard these pointers they should not be deleted like 
    bip::offset_ptr<FileIDManager> m_file_id_manager;
    bip::offset_ptr<TFileDataSet> m_files_data;
    bip::offset_ptr<TFileDescriptionSet> m_file_descriptions;
    bip::offset_ptr<Directory> m_directories;
    std::shared_ptr<bip::managed_mapped_file> m_seg;

    // load or construct File System storage
    void init_file_system();
};


#pragma once
#include "common.h"
#include "fileid_manager.h"
#include "file.h"

// Roles of Virtal File System Manager: - 
// create fs, remove fs, buckup fs, open fs,
// grow and shrink file system.

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


// TODO: add validation of format, existence of mapped file.
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

    // Top level functions
    void remove_storage();

    TFilePtr create_file(const char* name);

    TDirectoryPtr create_directory(const char* path);

    size_t write_file(const char* name, const char* buffer, size_t cb, size_t offset, size_t* written);
    size_t read_file(const char* name, char* buffer, size_t cb, size_t offset, size_t* readed);

    void remove_file(const char* file);
    void remove_directory(const char* dir_name);

    FileDescription get_file_desc(const char* name);
    vector<FileDescription> get_directory_list(const char* name);

    // inner functions
    void grow_storage();
    void remove_file_desc(TFileID id);
    bip::offset_ptr<FileDescription> add_file_desc(const char* name, const char* path, size_t size);
    void find_file_and_dir(const char* name, TDirectoryPtr* out_dir, TFilePtr* out_file);
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

    void init_file_system();
};


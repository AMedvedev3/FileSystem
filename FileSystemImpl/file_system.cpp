/**
@file
Root class for FileSystem implemantation
@author Alexey Medvedev
*/
#include "common.h"
#include <boost\thread.hpp>
#include <boost\interprocess\sync\named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>
#include "helper.h"
#include "fileid_manager.h"
#include "file_description.h"
#include "directory.h"
#include "file_system.h"
#include "file.h"
#include "exception.h"
#include "log.h"

using TMutex = bip::named_upgradable_mutex;
using TReadLock = bip::upgradable_lock<TMutex>;
using TWriteLock = bip::scoped_lock<TMutex>;

struct mutex_remove
{
    mutex_remove() { TMutex::remove("mm-{4BF8DF51-8133-4086-BB80-19BFFADE597C}"); }
    ~mutex_remove() { TMutex::remove("mm-{4BF8DF51-8133-4086-BB80-19BFFADE597C}"); }
} remover;

static TMutex mmutex(bip::open_or_create, "mm-{4BF8DF51-8133-4086-BB80-19BFFADE597C}");

void FileSystem::init_file_system()
{
    // get exclusive access
    TWriteLock write_lock(mmutex);

    assert(m_StorageName != NULL);
    m_seg.reset(new bip::managed_mapped_file(bip::open_or_create, m_StorageName, 20 * 1024 * 1024)); // "1Gb ought to be enough for anyone"
    assert(m_seg != nullptr);
    LF << "Storage \"" << m_StorageName << "\" successfully opened. " ;

    // File system objects grouped to different segments. 
    // That allows to decrease number of page loads when we works with same objects
    // for example: file descriptions grouped into segment allows to decrease cost of 
    // memory page loads when we traverse by files and directories
    // Also the file data placed into separate segment that allows to decrease 
    // a fragmentation of directory and 
    // in the future it have sense to make different segments for different size file data -
    // little files segment, big file segments (although it have sense to place large files into separate storage)

    m_file_id_manager = FileIDManager::load_or_create(m_seg);
    if (m_file_id_manager != nullptr)
    {
        LF << "FileIDManager successfully loaded." ;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("File ID Manager cannot be loaded."));
    }

    m_file_descriptions = m_seg->find_or_construct<TFileDescriptionSet>
        ("FILE_DESCRIPTIONS")
        (
            std::less<TFileID>(),
            TFileDataSet::allocator_type(m_seg->get_segment_manager())
        );

    if (m_file_descriptions != nullptr)
    {
        LF << "File descriptions segment successfully opened." ;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("File Description segnment cannot be loaded."));
    }

    m_directories = m_seg->find_or_construct<Directory>
        ("DIRECTORIES")
        (".", nullptr, *this);

    if (m_directories != nullptr)
    {
        LF << "Directories segment successfully opened." ;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Directories segnment cannot be loaded."));
    }
   
    m_files_data = m_seg->find_or_construct<TFileDataSet>
        ("FILES_DATA")
        (
            std::less<TFileID>(),
            TFileDataSet::allocator_type(m_seg->get_segment_manager())
            );

    if (m_files_data != nullptr)
    {
        LF << "Files data segment successfully opened." ;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Files Data segnment cannot be loaded."));
    }
    
    auto capacity = m_seg->get_free_memory();
    LF << "Storage capacity is " << capacity;
}

// TODO: make it protected. doesn't allow the public access as it doesnt have lock to prevent recursive lock 
// TODO: add hint to grow such as we can need to expand more then 100 Mb for new file
void FileSystem::grow_storage()
{
    m_seg->flush();
    m_seg.reset();
    bool result = bip::managed_mapped_file::grow(m_StorageName, 100 * 1024 * 1024);
    if (result == false)
    {
        LF << "Error: cannot grow the storage." ;
        throw std::exception("Error: cannot grow the storage.");
    }
    init_file_system();
    LF << "Storage successfully grown." ;
}

// Note: can throw bad_alloc. should be processed by caller
bip::offset_ptr<FileDescription> FileSystem::add_file_desc(const char* name, const char* path, size_t size)
{
    assert(name);
    TFileID id = m_file_id_manager->get_free_id();
    auto rc = m_file_descriptions->insert(std::make_pair(id, FileDescription(name, path, id, size)));
    if (rc.second)
    {
        LF << "File description id=" << id << " name: " << name << " successfully added to storage." ;

        return &rc.first->second;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("File Description cannot be created."));
    }

    LF << "Error at adding file description. id=" << id << " File name: " << name ;
    assert(rc.second);
    return NULL;
}

void FileSystem::remove_file_desc(TFileID id)
{ 
    m_file_descriptions->erase(id);
    m_file_id_manager->reg_deleted_file(id);
};


TFilePtr FileSystem::create_file(const char* name)
{
    if (!is_valid_path_name(name))
    {
        LF << "Error: invalid parameters. Invalid path string" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    ParsedPath parsed = ParsedPath::parse_file_path(name);

    if (parsed.directories.empty() ||
        !is_valid_file_name(parsed.fileName.c_str()))
    {
        LF << "Error: invalid parameters. Invalid file name" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_FILE_NAME)
            << errinfo_message("Invalid file name."));
    }

    // read lock
    // get upgradable access
    TReadLock read_lock(mmutex);
    
    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str() );
    if (dir == nullptr)
    {
        LF << "Error: Directory not found." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Directory not found."));
    }

    TFilePtr file  = dir->find_file(mkstr(parsed.fileName.c_str(), this->m_seg));
    if (file != nullptr)
    {
        LF << "Error: Such file already exist." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::FILE_ALREADY_EXIST)
            << errinfo_message("File already exist."));
    }

    // write lock
    // get exclusive access
    TWriteLock write_lock(boost::move(read_lock));

    TFileID id = m_file_id_manager->get_free_id();

    TFilePtr new_file = File::construct(parsed.fileName.c_str(), id, dir, *this);
    if (new_file == nullptr)
    {
        LF << "Error in file \"" << name << "\" creating." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::FILE_ALREADY_EXIST)
            << errinfo_message("File with that name already exists."));
    }

    LF << "File \"" << name << "\" succesffully created." ;

    return new_file;
}

TDirectoryPtr FileSystem::create_directory(const char* path)
{
    if (!is_valid_path_name(path))
    {
        LF << "Error: invalid parameters. Invalid path string" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }
    
    ParsedPath parsed = ParsedPath::parse_file_path(path);

    // read lock
    TReadLock read_lock(mmutex);
   
    TDirectoryPtr parent = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str());
    if(parent == NULL)
    {
        LF << "Error: path \"" << path << "\"not found." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Parent directory not found. Invalid path to directory."));
    }

    TDirectoryPtr same_dir = parent->find_sub_directory(mkstr(parsed.fileName.c_str(), this->m_seg));
    if (same_dir != nullptr)
    {
        LF << "Error: path \"" << path << "\" already exist." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_ALREADY_EXIST)
            << errinfo_message("Directory already exists."));
    }

    // write lock
    TWriteLock write_lock(boost::move(read_lock));

    TDirectoryPtr new_dir = parent->add_directory(*this, parsed.fileName.c_str());
    if (new_dir == NULL)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Directory cannot be created."));
    }
    return new_dir;
}

// returns how much bytes written to file
size_t FileSystem::write_file(const char* name, const char* buffer, size_t cb, size_t offset, size_t* written)
{
    assert(buffer);
    assert(cb);

    TDirectoryPtr dir;
    TFilePtr file;
    size_t last_pos;

    if (buffer == nullptr || cb == 0)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_PARAMETER)
            << errinfo_message("Buffer null or empty."));
    }

    // read lock
    TReadLock read_lock(mmutex);

    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        // write lock
        TWriteLock write_lock(boost::move(read_lock));

        last_pos = file->write(buffer, cb, offset, written, *this);
        assert(*written <= cb+ offset);
    }
    return last_pos;
}

size_t FileSystem::read_file(const char* name, char* buffer, size_t cb, size_t offset, size_t* readed)
{
    assert(buffer);
    assert(cb);

    TDirectoryPtr dir;
    TFilePtr file;
    size_t last_pos;

    if (buffer == nullptr || cb == 0)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_PARAMETER)
            << errinfo_message("Buffer null or empty."));
    }

    // read lock
    TReadLock read_lock(mmutex);

    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        last_pos = file->read(buffer, cb, offset, readed);
    }
    return last_pos;
}

// TODO: Implement MoveFile 
void move_file(const char* exsiting_file, const char* new_file)
{
    // TODO: find source dir and file
    // TODO: find destination dir
    // TODO: check that the destination dir doesn't contain file with same name 
    // TODO: change parent dir in file 
    // TODO: change name of file
}

// TODO: Implement CopyFile
void copy_file(const char* exsiting_file, const char* new_file)
{
    // TODO: find source dir and file
    // TODO: find destination dir
    // TODO: check that the destination dir doesn't contain file with same name
    // TODO: create new file in destination dir with new file name
    // TODO: copy file data
}

void FileSystem::remove_file(const char* name)
{
    TDirectoryPtr dir;
    TFilePtr file;
    // read lock
    TReadLock read_lock(mmutex);
    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        // write lock
        TWriteLock write_lock(boost::move(read_lock));
        file->clean(*this);
    }
}

void FileSystem::remove_directory(const char* dir_name)
{
    ParsedPath parsed = ParsedPath::parse_dir_path(dir_name);
    if(parsed.directories.size() == 1 && *parsed.directories.begin() == ".")
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("It is not allowed to remove root directory."));

    // read lock
    TReadLock read_lock(mmutex);

    TDirectoryPtr dir = find_directory(dir_name);
    if (dir->empty())
    {
        // write lock
        TWriteLock write_lock(boost::move(read_lock));

        TDirectoryPtr parent = dir->parent;
        parent->remove_directory(mkstr(dir->get_name(), *this));
        dir->clean(*this);
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_IS_NOT_EMPTY)
            << errinfo_message("Removing directory is not empty."));
    }

}

// throws exceptions if directory name is invalid or directory is not exists
TDirectoryPtr FileSystem::find_directory(const char* name)
{
    if (!is_valid_path_name(name))
    {
        LF << "Error: invalid parameters. Invalid path string" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    ParsedPath parsed = ParsedPath::parse_dir_path(name);

    if (parsed.directories.empty())
    {
        LF << "Error: invalid parameters. Invalid directory name." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str());
    if (dir == nullptr)
    {
        LF << "Error: Directory not found." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Directory not found."));
    }

    return dir;
}

// TODO: rework using find_directory
void FileSystem::find_file_and_dir(const char* name, TDirectoryPtr* out_dir, TFilePtr* out_file)
{
    if (!is_valid_path_name(name))
    {
        LF << "Error: invalid parameters. Invalid path string" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    ParsedPath parsed = ParsedPath::parse_file_path(name);

    if (parsed.directories.empty() ||
        !is_valid_file_name(parsed.fileName.c_str()))
    {
        LF << "Error: invalid parameters. Invalid file name" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_FILE_NAME)
            << errinfo_message("Invalid file name."));
    }

    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str());
    if (dir == nullptr)
    {
        LF << "Error: Directory not found." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Directory not found."));
    }

    TFilePtr file = dir->find_file(mkstr(parsed.fileName.c_str(), m_seg));
    if (file == nullptr)
    {
        LF << "Error: File not found." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::FILE_NOT_FOUND)
            << errinfo_message("File not found."));
    }

    *out_file = file;
    *out_dir = dir;
}


void FileSystem::remove_storage()
{
    // write lock
    TWriteLock write_lock(mmutex);
    if(!bip::file_mapping::remove(this->m_StorageName))
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Cannot drop current storage"));
}


FileDescription FileSystem::get_file_desc(const char* name)
{
    TDirectoryPtr dir;
    TFilePtr file;
    // read lock
    TReadLock read_lock(mmutex);
    find_file_and_dir(name, &dir, &file);
    // make a copy of description to make it thread safe
    return file->get_description();
}


vector<FileDescription> FileSystem::get_directory_list(const char* name)
{
    vector<FileDescription> desc_list;
    // read lock
    TReadLock read_lock(mmutex);

    TDirectoryPtr dir = this->find_directory(name);
    for (auto it : *dir->sub_directories.get())
        desc_list.push_back(it.second.get_description());
    for (auto it : *dir->files.get())
        desc_list.push_back(it.second.get_description());
    return desc_list;
}
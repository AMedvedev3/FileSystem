#include "common.h"
#include "helper.h"
#include "fileid_manager.h"
#include "file_description.h"
#include "directory.h"
#include "file_system.h"
#include "file.h"
#include "exception.h"

void FileSystem::init_file_system()
{
    // TODO: guard initialization
    //mutex_type::remove(MTX_NAME);
    //m_mutex = mutex_type(bip::open_or_create, MTX_NAME);
    //bip::scoped_lock<mutex_type> lock(m_mutex);
    assert(m_StorageName != NULL);
    m_seg.reset(new bip::managed_mapped_file(bip::open_or_create, m_StorageName, 1024 * 1024)); // "1Gb ought to be enough for anyone"
    assert(m_seg != nullptr);
    std::cout << "Storage \"" << m_StorageName << "\" successfully opened. " << std::endl;

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
        std::cout << "FileIDManager successfully loaded." << std::endl;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
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
        std::cout << "File descriptions segment successfully opened." << std::endl;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("File Description segnment cannot be loaded."));
    }

    m_directories = m_seg->find_or_construct<Directory>
        ("DIRECTORIES")
        (".", nullptr, *this);

    if (m_directories != nullptr)
    {
        std::cout << "Directories segment successfully opened." << std::endl;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
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
        std::cout << "Files data segment successfully opened." << std::endl;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Files Data segnment cannot be loaded."));
    }
    
    auto capacity = m_seg->get_free_memory();
    // TODO: make logging subsystem
    std::cerr << "Free space: " << (capacity >> 30) << "g\n";
}

// TODO: add hint to grow such as we can need to expand more then 200 Mb for new file
void FileSystem::grow_storage()
{
    m_seg->flush();
    // TODO: guard by mutex? mapped_file already contains mutex
    m_seg.reset();
    bool result = bip::managed_mapped_file::grow(m_StorageName, 200 * 1024 * 1024);
    if (result == false)
    {
        std::cout << "Error: cannot grow the storage." << std::endl;
        throw std::exception("Error: cannot grow the storage.");
    }
    init_file_system();
    std::cout << "Storage successfully grown." << std::endl;
}

// TODO: return result code
//void FileSystem::remove_file(TFileID id)
//{
//    auto result = m_files_data->erase(id);
//    if (result)
//    {
//        std::cout << "File data \"" << id << " successfully deleted." << std::endl;
//    }
//    result = m_file_descriptions->erase(id);
//    if (result)
//    {
//        std::cout << "File description \"" << id << " successfully deleted." << std::endl;
//    }
//    m_file_id_manager->reg_deleted_file(id);
//}

// Note: can throw bad_alloc. should be processed by caller
bip::offset_ptr<FileDescription> FileSystem::add_file_desc(const char* name, const char* path, size_t size)
{
    assert(name);
    TFileID id = m_file_id_manager->get_free_id();
    auto rc = m_file_descriptions->insert(std::make_pair(id, FileDescription(name, path, id, size)));
    if (rc.second)
    {
        std::cout << "File description id=" << id << " name: " << name << " successfully added to storage." << std::endl;

        return &rc.first->second;
    }
    else
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("File Description cannot be created."));
    }

    std::cout << "Error at adding file description. id=" << id << " File name: " << name << std::endl;
    assert(rc.second);
    return NULL;
}

void FileSystem::remove_file_desc(TFileID id)
{ 
    m_file_descriptions->erase(id);
    m_file_id_manager->reg_deleted_file(id);
};

// TODO: return error code 
//void FileSystem::add_file_desc(TFileID id, uintmax_t size, const char* path, const char* name)
//{
//    assert(name);
//    assert(size);
//    assert(id);
//    assert(path);
//
//    // TODO: fill attributes and dates
//    FileDescription fdesc(name, path, id, size);
//    auto rc = this->m_file_descriptions->insert(std::make_pair(id, fdesc));
//    if (rc.second)
//    {
//        std::cout << "File description id=" << id << " name: " << name << " successfully added to storage." << std::endl;
//    }
//    else
//    {
//        std::cout << "Error at adding file description. id=" << id << " File name: " << name << std::endl;
//    }
//    assert(rc.second);
//}

//void FileSystem::add_file_data(TFileID id, uintmax_t size)
//{
//    // TODO: make it TLS located
//    static int recursion = 0;
//    try
//    {
//        auto fileData = shared_vector<char>(size, 'K', GetFSAllocator());
//        m_filesData->insert(std::make_pair(id, fileData));
//    }
//    catch (bip::bad_alloc&)
//    {
//        this->grow_storage();
//        // retry this three times
//        if (recursion++ < 3)
//        {
//            add_file_data(id, size);
//        }
//        // TODO: throw exception after three times
//    }
//    recursion = 0;
//}



// TODO: add logging
TFilePtr FileSystem::create_file(const char* name)
{
    // TODO: validate logical directory structure (existance of path, file existance)
    // TODO: return error code if this function will be top level
    // TODO: check full path length
    // TODO: enclose first part into Open() function

    if (!is_valid_path_name(name))
    {
        std::cout << "Error: invalid parameters. Invalid path string" << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    ParsedPath parsed = ParsedPath::parse_file_path(name);

    if (parsed.directories.empty() ||
        !is_valid_file_name(parsed.fileName.c_str()))
    {
        std::cout << "Error: invalid parameters. Invalid file name" << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_FILE_NAME)
            << errinfo_message("Invalid file name."));
    }
    
    // TODO: place read lock here!!!

    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str() );
    if (dir == nullptr)
    {
        std::cout << "Error: Directory not found." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Directory not found."));
    }

    TFilePtr file  = dir->find_file(mkstr(parsed.fileName.c_str(), this->m_seg));
    if (file != nullptr)
    {
        std::cout << "Error: Such file already exist." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::FILE_ALREADY_EXIST)
            << errinfo_message("File already exist."));
    }

    TFileID id = m_file_id_manager->get_free_id();

    // TODO: place write lock here!!!
    
    // TODO: handle inner exception here?
    TFilePtr new_file = File::construct(parsed.fileName.c_str(), id, dir, *this);
    if (new_file == nullptr)
    {
        std::cout << "Error in file \"" << name << "\" creating." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::FILE_ALREADY_EXIST)
            << errinfo_message("File with that name already exists."));
    }

    std::cout << "File \"" << name << "\" succesffully created." << std::endl;

    return new_file;
}

TDirectoryPtr FileSystem::create_directory(const char* path)
{
    // TODO: enclose first part into file/directory Open function()
    if (!is_valid_path_name(path))
    {
        std::cout << "Error: invalid parameters. Invalid path string" << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }
    
    ParsedPath parsed = ParsedPath::parse_file_path(path);

    // TODO: place read lock here!!!
   
    TDirectoryPtr parent = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str());
    if(parent == NULL)
    {
        std::cout << "Error: path \"" << path << "\"not found." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Parent directory not found. Invalid path to directory."));
    }

    TDirectoryPtr same_dir = parent->find_sub_directory(mkstr(parsed.fileName.c_str(), this->m_seg));
    if (same_dir != nullptr)
    {
        std::cout << "Error: path \"" << path << "\" already exist." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::DIRECTORY_ALREADY_EXIST)
            << errinfo_message("Directory already exists."));
    }

    // TODO: place write lock here!!!

    TDirectoryPtr new_dir = parent->add_directory(*this, parsed.fileName.c_str());
    if (new_dir == NULL)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::STORAGE_STRUCTURE_ERROR)
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
            << errinfo_rpc_code(fs_error::INVALID_PARAMETER)
            << errinfo_message("Buffer null or empty."));
    }

    // TODO: place read lock here!!!

    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        // TODO: place write lock here!!!

        last_pos = file->write(buffer, cb, offset, written, *this);
        assert(*written <= cb+ offset);
    }
    return last_pos;
}

size_t FileSystem::read_file(const char* name, char* buffer, size_t cb, size_t offset, size_t* readed)
{
    // TODO: find source dir and file
    // TODO: call read file
    assert(buffer);
    assert(cb);

    TDirectoryPtr dir;
    TFilePtr file;
    size_t last_pos;

    if (buffer == nullptr || cb == 0)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_PARAMETER)
            << errinfo_message("Buffer null or empty."));
    }

    // TODO: place read lock here!!!

    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        // TODO: place write lock here!!!
        last_pos = file->read(buffer, cb, offset, readed);
        assert(*readed != cb);
    }
    return last_pos;
}

void move_file(const char* exsiting_file, const char* new_file)
{
    // TODO: find source dir and file
    // TODO: find destination dir
    // TODO: check that the destination dir doesn't contain file with same name 
    // TODO: change parent dir in file 
    // TODO: change name of file
}

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
    find_file_and_dir(name, &dir, &file);
    if (dir != nullptr && file != nullptr)
    {
        file->clean(*this);
    }
}

void remove_directory(const char* dir)
{
    // TODO: find directory
    // TODO: check that it is empty
    // TODO: remove directory from parent directory list
}

// TODO: Make the same - find directory or maybe top level find_directory and find_file

void FileSystem::find_file_and_dir(const char* name, TDirectoryPtr* out_dir, TFilePtr* out_file)
{
    if (!is_valid_path_name(name))
    {
        std::cout << "Error: invalid parameters. Invalid path string" << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Invalid directory name."));
    }

    ParsedPath parsed = ParsedPath::parse_file_path(name);

    if (parsed.directories.empty() ||
        !is_valid_file_name(parsed.fileName.c_str()))
    {
        std::cout << "Error: invalid parameters. Invalid file name" << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::INVALID_FILE_NAME)
            << errinfo_message("Invalid file name."));
    }

    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, parsed.get_path().c_str());
    if (dir == nullptr)
    {
        std::cout << "Error: Directory not found." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::DIRECTORY_NOT_FOUND)
            << errinfo_message("Directory not found."));
    }

    TFilePtr file = dir->find_file(mkstr(parsed.fileName.c_str(), m_seg));
    if (file == nullptr)
    {
        std::cout << "Error: File not found." << std::endl;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_rpc_code(fs_error::FILE_NOT_FOUND)
            << errinfo_message("File not found."));
    }

    *out_file = file;
    *out_dir = dir;
}

// TODO: use  
bool FileSystem::is_directory_exists(const char* path)
{
    if (!is_valid_path_name(path))
    {
        assert(0);
        // Log error
        return FALSE;
    }

    TDirectoryPtr dir = Directory::find_directory(*this, this->m_directories, path);
    return dir != NULL;
}


bool FileSystem::is_file_exists(const char* path)
{
    return is_valid_file_name(path);
}


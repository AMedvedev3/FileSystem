// TestFileSys.cpp : Defines the entry point for the console application.
//

#include <Windows.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/container/scoped_allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/date_time/date.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>

const char* MTX_NAME = "mmtx-5DB432C6-245D-49A2-996D-D077E2B3FF5F";
namespace bip = boost::interprocess;


using mutex_type = bip::named_mutex;
using TFileID = uintmax_t;
template <typename T> using shared_alloc = bip::allocator<T, bip::managed_mapped_file::segment_manager>;
template <typename T> using shared_vector = boost::container::vector<T, shared_alloc<T> >;
template <typename K, typename V, typename P = std::pair<K, V>, typename Cmp = std::less<K> >
using shared_map = boost::container::flat_map<K, V, Cmp, shared_alloc<P> >;
template <typename T> using shared_set = boost::container::set<T, std::less<T>, shared_alloc<T> >;
template <typename T> using shared_list = boost::container::list<T, shared_alloc<T> >;
using shared_string = bip::basic_string<char, std::char_traits<char>, shared_alloc<char> >;

//class Header
//{
//#define HEADER_SIGNATURE 0x31937A98D
//public:
//    Header() : signature(HEADER_SIGNATURE), length(0)
//    {}
//
//    uint64_t signature;
//    uintmax_t length;
//    uintmax_t lastFileID;
//};

// TODO: try to rework path and name to shared string
struct FileDescription
{
    FileDescription()
    {
    }

    FileDescription(const char* name, const char* path, 
        const std::shared_ptr<bip::managed_mapped_file>& seg)
        :fileID(0), size(0)
    {
        if (name)
            strcpy_s(this->name, name);
        if (path)
            strcpy_s(this->path, path);
    }

    FileDescription(const char* name, const char* path, 
        TFileID id, uintmax_t size, 
        const std::shared_ptr<bip::managed_mapped_file>& seg
        )
        :fileID(id), size(size)
    {
        if (name)
            strcpy_s(this->name, name);
        if (path)
            strcpy_s(this->path, path);
    }

    TFileID fileID;
    char name[256];
    char path[256];
    uintmax_t size;
    //boost::date_time:: createdDate;
    //boost::date_time::date changedDate;
    // chrono createdDate
    // chrono changedDate
    // bitflag attribs

};


struct UserGroup
{
    std::string name;
};
//
class DirectoryEntry;
//using TShm = bip::managed_mapped_file::segment_manager;
using TFileDataSet = shared_map< TFileID, shared_vector<char> >;
using TFileSet = shared_map< TFileID, FileDescription >;
using TDirectorySet = shared_map< shared_string, DirectoryEntry >;
using TDirFileSet = shared_set<TFileID>;
using TDirectoryEntry_ptr = bip::managed_shared_ptr<DirectoryEntry, bip::managed_mapped_file>::type;

// TODO: move to separate module Directories.hpp
// Element of directory
class DirectoryEntry
{
public:

    DirectoryEntry()
    {}

    DirectoryEntry(DirectoryEntry& arg)
    {
        if (this != &arg)
        {
            *this = arg;
        }
    }

    // TODO: implement classically
    DirectoryEntry& operator =(DirectoryEntry arg)
    {
        this->m_parent = arg.m_parent;
        this->m_seg = arg.m_seg;
        this->m_directoryDesc = this->m_directoryDesc;
        this->m_sub_directories = this->m_sub_directories;
        return *this;
    }

    DirectoryEntry(const std::shared_ptr<bip::managed_mapped_file>& seg,
        DirectoryEntry* parent,
        char* name):
        m_seg(seg),
        m_directoryDesc(name, NULL, seg)
    {
        assert(parent);
        assert(name);

        m_parent = bip::make_managed_shared_ptr(
            m_seg->construct<DirectoryEntry>(bip::anonymous_instance)(*parent), *m_seg);
            
        m_sub_directories = m_seg->construct<TDirectorySet>(bip::anonymous_instance)
            (
                std::less<shared_string>(),
                TDirectorySet::allocator_type(m_seg->get_segment_manager())
            );

        m_files = m_seg->construct<TDirFileSet>(bip::anonymous_instance)
            (
                std::less<TFileID>(),
                TDirFileSet::allocator_type(m_seg->get_segment_manager())
            );
    }

    
     //iterator of directory entries in this directory
    // TODO: set ordered by name?
    // ??????  TODO: 
    // shared_map< TFileID, DiectoryEntry > entries;

    // TODO: validate 
    void add_file(TFileID id)
    {
        m_files->insert(id);
    }

    void remove_file(TFileID id)
    {
        m_files->erase(id);
    }

    void create_sub_directory(char* name)
    {
        DirectoryEntry new_directory(m_seg, this, name);
        m_sub_directories->insert(
            std::make_pair(
                shared_string(name, m_seg->get_segment_manager()),
                new_directory
            ));
    }

    void delete_sub_directory(char* name)
    {
        // TODO: check that directory is empty

    }
      
protected:
   
    TDirectoryEntry_ptr                              m_parent;
    TDirectorySet*                                   m_sub_directories;
    TDirFileSet*                                       m_files;
    FileDescription                                 m_directoryDesc; // in the future, when
    std::shared_ptr<bip::managed_mapped_file> m_seg;
};


struct mutex_remove
{
    mutex_remove() { mutex_type::remove(MTX_NAME); }
    ~mutex_remove() { mutex_type::remove(MTX_NAME); }
} remover;

//static mutex_type mutex(bip::open_or_create, MTX_NAME);

// TODO: move to separate module
class FileIDManager;
using TFreeFileIds = shared_list<TFileID>;

// TODO: single instance per FileSystem, prevent copying, creating, etc.
class FileIDManager
{
public:

    TFileID get_free_id()
    {
        TFileID id = 0;
        if (!m_free_ids->empty())
        {
            id = *m_free_ids->begin();
            m_free_ids->pop_front();
        }
        else
        {
            id = ++m_last_free_id;
        }
        return id;
    }
    

    // TODO: make the test to check last Id
    void reg_deleted_file(TFileID id)
    {
        // check that removed last file in the list then just decrease m_last_free_id
        if (id != m_last_free_id - 1)
            m_free_ids->push_back(id);
        else
            m_last_free_id--;
    }

    static FileIDManager* load_or_create (const std::shared_ptr<bip::managed_mapped_file>& seg)
    {
        FileIDManager* file_id_manager  = seg->find_or_construct <FileIDManager>("FileIDManager")();

        file_id_manager->m_free_ids =
             seg->find_or_construct<TFreeFileIds>
            ("FreeFileIDList")
            (
            TFreeFileIds::allocator_type(seg->get_segment_manager())
            );

        return  file_id_manager;
    }
    
protected:
    TFileID m_last_free_id;
    TFreeFileIds* m_free_ids;
};


// Roles of Virtal File System Manager: - 
// create fs, remove fs, buckup fs, open fs,
// grow and shrink file system.
//
// Roles of Directory Service - find file, 
// enumerate directories and files, navigation by directories,
// control logical place integrity of directories and files,
// move file, remove file.
//
// Roles of File - open file, write file, 
// under question operation - create file, remove file, move file.
//
// Roles of FileDescription - identify file by name, by full path, 
// create, store and update metadata information about file


// TODO: add validation of format, existence of mapped file.
// TODO: make the segment by offset of standard Header of FS file.

// TODO: incapsulate hidden members  
// TODO: incapsulate FileData into separate class
class FileSystem
{
public:
    //const char* m_StorageName = "./demo1.bin";

    FileSystem(): 
        m_StorageName("./demo1.bin"),
        m_mutex(bip::open_or_create, MTX_NAME),
        m_files(NULL), m_filesData(NULL)
    {
        init_file_system();
    };

    FileSystem(char* storageName):
        m_StorageName(storageName),
        m_mutex(bip::open_or_create, MTX_NAME),
        m_files(NULL), m_filesData(NULL)
    {
        init_file_system();
    }

    virtual ~FileSystem() 
    {
        try
        {
            shrink_storage();
        }
        catch(...)
        {
            std::cout << "Error at shrinking storage at finishing work." << std::endl;
        }
        mutex_type::remove(MTX_NAME);
    };

    const auto GetFSAllocator() 
    {
        assert(m_filesData);
        return m_filesData->get_allocator().get_segment_manager();
    };
       
    // TODO:  test, will be moved to File class
    void add_file_data(TFileID id, uintmax_t size);
    void add_file_desc(TFileID id, uintmax_t size, const char* path, const char* name);

    // TODO: add logging
    TFileID create_file(const char* path, const char* name)
    {
        // TODO: validate the path and file name symbols, name and path length
        // TODO: check that path and name are not null or empty
        // TODO: validate logical directory structure (existance of path, file existance)
        // TODO: return error code if this function will be top level

        const uintmax_t default_size = 2 * 1024; // below page size
        // TODO: below is the transaction target too
        TFileID id = m_file_id_manager->get_free_id();
        
        try
        {
            // TODO: maybe return pointer to allocated data for transaction work
            add_file_data(id, default_size);
            add_file_desc(id, default_size, path, name);
            std::cout << "File \"" << name << "\" succesffully created." << std::endl;

        }
        catch (bip::bad_alloc&)
        {
            grow_storage();
            id = create_file(path, name);
        }
                                
        return id;
    }

    // TODO: return result code
    void remove_file(TFileID id)
    {
        auto result = m_filesData->erase(id);
        if (result)
        {
            std::cout << "File data \"" << id << " successfully deleted." << std::endl;
        }
        result = m_files->erase(id);
        if (result)
        {
            std::cout << "File data \"" << id << " successfully deleted." << std::endl;
        }
        m_file_id_manager->reg_deleted_file(id);
    }

    // TODO: add hint to grow such as we can need to expand more then 200 Mb for new file
    void grow_storage()
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

    void shrink_storage()
    {
        m_seg->flush();
        // TODO: guard by mutex? mapped_file already contains mutex
        m_seg.reset();
        bool result = bip::managed_mapped_file::shrink_to_fit(m_StorageName);
        if (result == false)
        {
            std::cout << "Error: cannot shrink the storage." << std::endl;
            throw std::exception("Error: cannot shrink the storage.");
        }
        init_file_system();
        std::cout << "Storage successfully shrinked." << std::endl;
    }

    void open_storage()
    {
        init_file_system();
    }

//protected:
    mutex_type m_mutex;
    const char* m_StorageName;

    // TODO: enclose this set of datas into recreating wrapper object
    // TODO: guard these pointers they should not be deleted like 
    TFileDataSet* m_filesData;
    TFileSet* m_files;
    std::shared_ptr<bip::managed_mapped_file> m_seg;
    FileIDManager* m_file_id_manager;
    DirectoryEntry* m_directoryRoot;
        
    void init_file_system();
};

 void FileSystem::init_file_system()
{
    // TODO: guard initialization
    //mutex_type::remove(MTX_NAME);
    //m_mutex = mutex_type(bip::open_or_create, MTX_NAME);
    //bip::scoped_lock<mutex_type> lock(m_mutex);
    assert(m_StorageName != NULL);
    m_seg.reset( new bip::managed_mapped_file(bip::open_or_create, m_StorageName, 10*1024*1024)); // "1Gb ought to be enough for anyone"
    assert(m_seg != NULL);
    std::cout << "Storage \"" << m_StorageName << "\" successfully opened. " << std::endl;
    
    m_file_id_manager = FileIDManager::load_or_create(m_seg);
    if (m_file_id_manager != NULL)
    {
        std::cout << "FileIDManager successfully loaded." << std::endl;
    }

    m_files = m_seg->find_or_construct<TFileSet>
        ("FILES")
        (
            std::less<TFileID>(),
            TFileSet::allocator_type(m_seg->get_segment_manager())
            );

    if (m_files != NULL)
    {
        std::cout << "Files segment successfully opened." << std::endl;
    }

    m_filesData = m_seg->find_or_construct<TFileDataSet>
        ("FILES_DATA")
        (
            std::less<TFileID>(),
            TFileDataSet::allocator_type(m_seg->get_segment_manager())
        );

    if (m_filesData != NULL)
    {
        std::cout << "Files data segment successfully opened." << std::endl;
    }

    m_directoryRoot = m_seg->find_or_construct<DirectoryEntry>
        ("DIRECTORY") ();
    if (m_directoryRoot = NULL)
    {
        std::cout << "Directory segment successfully opened." << std::endl;
    }
    
    auto capacity = m_seg->get_free_memory();
    // TODO: make logging subsystem
    std::cerr << "Free space: " << (capacity >> 30) << "g\n";
}

 // TODO: return error code 
 void FileSystem::add_file_desc(TFileID id, uintmax_t size, const char* path, const char* name)
 {
     assert(name);
     assert(size);
     assert(id);
     assert(path);

     // TODO: fill attributes and dates
     //FileDescription fdesc(name, path, id, size, m_seg);
     //auto rc = m_files->insert(std::make_pair(id, fdesc));
     //if (rc.second)
     //{
     //    std::cout << "File description id=" << id << " name: " << name << " successfully added to storage." << std::endl;
     //}
     //else
     //{
     //    std::cout << "Error at adding file description. id=" << id << " File name: " << name << std::endl;
     //}
     //assert(rc.second);
 }

 void FileSystem::add_file_data(TFileID id, uintmax_t size)
 {
     // TODO: make it TLS located
     static int recursion = 0;
     try
     {
         auto fileData = shared_vector<char>(size, 'K', GetFSAllocator());
         m_filesData->insert(std::make_pair(id, fileData));
     }
     catch (bip::bad_alloc&)
     {
         this->grow_storage();
         // retry this three times
         if (recursion++ < 3)
         {
             add_file_data(id, size);
         }
         // TODO: throw exception after three times
     }
     recursion = 0;
 }

 void TestCreateFillStorage()
 {
     // TODO: rework this debris
     FileSystem fs("./demo1.bin");
     //bip::scoped_lock<mutex_type> lock(fs.m_mutex);
     auto fsalloc = fs.GetFSAllocator();

     std::cout << fs.m_filesData->size() << '\n';

     for (uintmax_t i = 0; i < 1'000'000; ++i)
     {
         auto freeMemory = fs.m_seg->get_free_memory();
         auto allocatingMemory = size_t(rand() % (20'000'000));
         std::cout << "free memory: " << fs.m_seg->get_free_memory() << std::endl;
         //if (freeMemory < allocatingMemory || freeMemory < 300*1024)
         //{
         //    std::cout << "Error: not enough memory for allocation. Resize file system file" << std::endl;
         //    fs.grow_storage();
         //    //break;
         //}
         fs.add_file_data(i, allocatingMemory);
         ::Sleep(0);
     }

     fs.shrink_storage();
     //fs.m_seg->flush();
     //delete fs.m_seg;
 }


 void TestRandomFileDelete()
 {
     // open file system
     FileSystem fs("./demo1.bin");

     for (int i = 0; i < 100; i++)
     {
         TFileID id = rand() % 1'000'000;
         // TODO: incapsulate to FileRemove()
         if (fs.m_filesData->erase(id) != 1)
         {
             std::cout << "File data id=" << id << " not found." << std::endl;
         }
         if (fs.m_files->erase(id) != 1)
         {
             std::cout << "File id=" << id << " not found" << std::endl;
         }
     }
 }

 void TestRemoveCreateFiles()
 {
     FileSystem fs;
     for (int i = 1; i < 101; i++)
     {
         fs.remove_file(i);
         std::ostringstream ss;
         ss << "TestFile" << i;
         fs.create_file("./root/", ss.str().append(".txt").c_str());
     }
 }

 void TestPrintFilesInfo(TFileID first, TFileID last)
 {
     assert(first > 0);
     assert(last >= first);
     FileSystem fs;
     for (TFileID id = first; id < last + 1; id++)
     {
         auto pair = fs.m_files->find(id);
         if (pair != fs.m_files->end())
         {
             const FileDescription& fds = pair->second;
             std::cout << "File id=" << fds.fileID << " name:  " << fds.name;
             std::cout << " path: " << fds.path << " size=" << fds.size << std::endl;
         }
     }
 }

 void TestDirectory()
 {
     
 }

int main()
{
    //TestRandomFileDelete();
    //TestRemoveCreateFiles();
    TestPrintFilesInfo(1, 100);
    
}
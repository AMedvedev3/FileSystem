/**
@file
Unit tests console app for testing FileSystem implementation in details
@author Alexey Medvedev
*/
#include <Windows.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <exception>

#include "common.h"
#include "helper.h"
#include "exception.h"
#include "file_system.h"
#include "directory.h"
#include "file_description.h"
#include "fileid_manager.h"
#include "file.h"
#include "log.h"

namespace bip = boost::interprocess;

 void TestPrintFilesInfo(TFileID first, TFileID last)
 {
     assert(first > 0);
     assert(last >= first);
     FileSystem fs;
     for (TFileID id = first; id < last + 1; id++)
     {
         auto pair = fs.m_file_descriptions->find(id);
         if (pair != fs.m_file_descriptions->end())
         {
             const FileDescription& fds = pair->second;
             LF << "File id=" << fds.fileID << " name:  " << fds.name;
             LF << " path: " << fds.path << " size=" << fds.size ;
         }
     }
 }

 
 void PrintDirectories(TDirectoryPtr& dir, int tabs=0)
 {
     auto it = dir->sub_directories->begin();
     auto end = dir->sub_directories->end();
     if (tabs == 0)
         LF << "=========== directoriy list begin =========" ;
     for (; it != end; ++it)
     {
         TDirectoryPtr tmp(&it->second);
         for (int i = 0; i < tabs; i++)
             LF << "\t";
         LF << "Directory \"" << tmp->get_name();
         LF << "\" subdirectories: " << tmp->sub_directories->size();
         auto parent = tmp->parent;
         if (parent != nullptr)
         {
             LF << " parent: \"" << parent->get_name() << "\"";
         }
         LF ;
         tabs++;
         PrintDirectories(tmp, tabs);
         tabs--;
     }
     if(tabs == 0)
         LF << "=========== directoriy list end =========" ;
 }

 void TestDirectory()
 {
     {
         FileSystem fs;
         const char* test = "TestDirectory";
         // make short form - mks("text", fs)
         shared_string stest = shared_string(test, fs.m_seg->get_segment_manager());

         fs.m_directories->add_directory(fs, test);
         PrintDirectories(fs.m_directories);

         fs.m_directories->add_directory(fs, test);
         PrintDirectories(fs.m_directories);

         fs.m_directories->remove_directory(stest);
         PrintDirectories(fs.m_directories);

         fs.m_directories->add_directory(fs, test);
         PrintDirectories(fs.m_directories);
     }
     {
         FileSystem fs;
         LF << "Root directory \"" << "" << fs.m_directories->get_name() << "\" exists." ;
         const char* test = "TestDirectory";
         PrintDirectories(fs.m_directories);

         shared_string stest = shared_string(test, fs.m_seg->get_segment_manager());
         auto dir = fs.m_directories->find_sub_directory(stest);
         auto new_dir = dir->add_directory(fs, "SubDirectory");
         PrintDirectories(fs.m_directories);
     }
 }

 void TestDirectory2()
 {
     {
         FileSystem fs;
         const char* test = "TestDirectory";
         shared_string stest = mkstr(test, fs);

         fs.m_directories->add_directory(fs, test);

         fs.m_directories->add_directory(fs, "test1");
         PrintDirectories(fs.m_directories);

         fs.m_directories->add_directory(fs, "test2");
         PrintDirectories(fs.m_directories);

         auto dir = fs.m_directories->add_directory(fs, "test3");
         PrintDirectories(fs.m_directories);
         dir->add_directory(fs, "test11");
         dir->add_directory(fs, "test12");
         dir->add_directory(fs, "test13");
         dir->add_directory(fs, "test14");
         dir->add_directory(fs, "test15");
         PrintDirectories(fs.m_directories);
     }
     {
         FileSystem fs;
         LF << "Root directory \"" << "" << fs.m_directories->get_name() << "\" exists." ;
         const char* test = "TestDirectory";
         PrintDirectories(fs.m_directories);
         shared_string stest = mkstr(test, fs);
         auto dir = fs.m_directories->find_sub_directory(stest);
         if (dir != nullptr)
         {
             auto new_dir = dir->add_directory(fs, "SubDirectory");
             assert(new_dir);
             std::string s = new_dir->get_name();
             assert(new_dir->parent);
             std::string s1 = new_dir->parent->get_name();
         }
         else
         {
             LF << "Error: created directory didn't found after storage reopened." ;
             assert(0);
         }
         PrintDirectories(fs.m_directories);
     }
 }

 int TestHelper()
 {
     FileSystem fs;
     // test shared_string make helper
     if (mkstr("test", fs) != "test")
         return 1;
     if (mkstr("test", fs.m_seg) != "test")
         return 2;

     // test path parser
     ParsedPath res = ParsedPath::parse_dir_path("");
     if (!res.directories.empty() && !res.fileName.empty())
         return 3;
     res = ParsedPath::parse_dir_path(NULL);
     if (!res.directories.empty() && !res.fileName.empty())
         return 4;
     res = ParsedPath::parse_dir_path("//test/test///test\\\\test/");
     if (res.directories.size() != 4 && !res.fileName.empty())
         return 5;
     res = ParsedPath::parse_file_path("test/test/test/file.txt");
     if (res.directories.size() != 3 && res.fileName != "file.txt")
         return 6;
     res = ParsedPath::parse_file_path("test/te.,st/test/file.txt");
     if (res.directories.size() != 3 && res.fileName != "file.txt")
         return 7;

     // test valid file name
     bool bres = is_valid_file_name(NULL);
     if (bres)
         return 8;
     bres = is_valid_file_name("");
     if (bres)
         return 9;
     bres = is_valid_file_name("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
        fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
        ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
        ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
        fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
        ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
     if (bres)
         return 10;
     // valid name
     bres = is_valid_file_name("gahdhhdjjsjskkdllls;;hhsj()----gsghhdj%GGGGFFsss+++++_____.-----");
     if (!bres)
         return 11;
     bres = is_valid_file_name(".");
     if (bres)
         return 12;
     bres = is_valid_file_name(" test.txt");
     if (bres)
         return 13;
     bres = is_valid_file_name("test.txt   ");
     if (bres)
         return 14;
     bres = is_valid_file_name("\1\2\31");
     if (bres)
         return 15;
     bres = is_valid_file_name("te<st>.txt");
     if (bres)
         return 16;
     bres = is_valid_file_name("test:.txt");
     if (bres)
         return 17;


     // TODO: add test valid path 
     return 0;
 }

 void CreateDirectories(FileSystem& fs, TDirectoryPtr dir, int level=1, int max_level=5 )
 {
     assert(dir);
     assert(level < max_level +2 );
     if (level <= max_level)
     {
         for (int i = 1; i <= 5; i++)
         {
             ostringstream ostr;
             ostr << "Dir" << i << "L" << level;
             auto tmp = dir->add_directory(fs, ostr.str().c_str());
             level++;
             CreateDirectories(fs, tmp, level, max_level);
             level--;
         }
     }
 }

 int TestCreateDirectoriesTree()
 {
     // make five levels of directory tree
     std::string search = "/./Dir1L1/Dir2L2/Dir1L3/";
     {
         FileSystem fs;
         CreateDirectories(fs, fs.m_directories, 1, 4);
         PrintDirectories(fs.m_directories);
     }
     {
         FileSystem fs;
         TDirectoryPtr dir = Directory::find_directory(fs, fs.m_directories, search.c_str());
         assert(dir);
         assert(dir->parent);
         assert(dir->description);
         assert(dir->files);
         assert(dir->sub_directories);
         assert(!dir->sub_directories->empty());
         //assert(dir->description->fileID != 0);
         assert(!std::string(dir->description->name).empty());
         LF << "Found directory \"" << dir->get_name() << "\"" ;
         std::string path = dir->get_path();
         LF << "Found directory path is \"" << path << "\"" ;
         if (search != path)
             return 1;
     }
     return 0;
 }

 int TestTopLevelCreateDirectory()
 {
     std::string path = "./\\test_directory1//";
     {
         FileSystem fs;
         TDirectoryPtr dir;
         for (int i = 2; i < 10; i++)
         {
             dir = fs.create_directory(path.c_str());
             if (dir == NULL)
                 return i + 1;
             LF << "Dir created \"" << dir->get_path() << "\"" ;
             path = dir->get_path().append("/test_directory").append(std::to_string(i));
         }
         path = dir->get_path();
         PrintDirectories(fs.m_directories);
     }
     {
         // check storage
         FileSystem fs;
         PrintDirectories(fs.m_directories);

         TDirectoryPtr dir = Directory::find_directory(fs, fs.m_directories, path.c_str());
         if (dir == NULL)
             return 20;
         LF << "Found directory \"" << dir->get_path() << "\"" ;
         if (dir->get_name() == NULL)
             return 21;
         if (dir->get_path() != path)
             return 22;
     }
     return 0;
 }


 int TestTopLevelCreateFile()
 {
     const char* filePath = "/./test_directory1/test_directory2/test_directory3/test_directory4/test.txt";
     const char* name = "test.txt";
     {
         FileSystem fs;
         TFilePtr file = fs.create_file(filePath);
         if (file == NULL)
             return 1;
         // check description refernce
         if (file->get_name() != name)
             return 2;
         // check directory reference
         if (file->get_path() != filePath)
             return 3;
         // check consistency
         if (file->get_size() != 0)
             return 4;
         TDirectoryPtr dir = file->get_directory();
         if (dir == NULL)
             return 5;
         // find file in parent directory
         file = dir->find_file(mkstr(file->get_name().c_str(), fs));
         if (file == NULL || file->get_name() != name)
             return 6;
         // find the file in the storage
         // check consistence of file structure
         auto id  = file->get_id();
         auto n = file->get_name();
         auto p = file->get_path();
         auto s = file->get_size();
         auto it = fs.m_file_descriptions->find(file->get_id());
         if (it == fs.m_file_descriptions->end())
             return 7;
         auto it2 = fs.m_files_data->find(file->get_id());
         if (it2 == fs.m_files_data->end())
             return 8;
         char teststr[] = "Test sring writing to file!!!";
         size_t written;
         size_t last_pos = fs.write_file(filePath, teststr, sizeof(teststr) - 1 , 0, &written);
         last_pos = fs.write_file(filePath, teststr, sizeof(teststr) - 1 , last_pos, &written);
         if (file->get_size() != (sizeof(teststr) - 1) * 2)
             return 9;
     }
     // test write file
     {
         char teststr[] = "Test sring writing to file!!!";
         FileSystem fs;  
         size_t written;
         size_t last_pos = fs.write_file(filePath, teststr, sizeof(teststr), 0, &written);
         if (written != sizeof(teststr))
             return 20;
     }
     // test read file
     {
         char teststr[] = "Test sring writing to file!!!";
         char buffer[512];
         memset(buffer, 0, sizeof(buffer));
         FileSystem fs;
         size_t readed;
         fs.read_file(filePath, buffer, sizeof(buffer), 0, &readed);
         if (readed >= sizeof(buffer))
             return 30;
     }
     // test remove file
     {
         FileSystem fs;
         const char* dirPath = "/./test_directory1/test_directory2/test_directory3/test_directory4/";
         // check that file exists before removing
         TDirectoryPtr dir = Directory::find_directory(fs, fs.m_directories, dirPath);
         if (dir == nullptr)
             return 40;
         TFilePtr file =  dir->find_file(mkstr(name, fs));
         if (file == nullptr)
             return 41;
         TFileID id = file->get_id();
         if (id == 0)
             return 42;
         // remove file
         fs.remove_file(filePath);
         // check that no one part of file stays in the file system
         if (dir->find_file(mkstr(name, fs)) != nullptr)
             return 43;
         auto it1 = fs.m_files_data->find(id);
         if (it1 != fs.m_files_data->end())
             return 44;
         auto it2 = fs.m_file_descriptions->find(id);
         if (it2 != fs.m_file_descriptions->end())
             return 45;
     }
     return 0;
 }


 int TestTopLevelRemoveDirectories()
 {
     FileSystem fs;
     const char* directoryName = "./TestRemoveDirectory/";
     TDirectoryPtr dir  = fs.create_directory(directoryName);
     if (dir == nullptr)
         return 1;
     const char* dname = dir->get_name();
     if (std::string(dir->get_name()) != "TestRemoveDirectory")
         return 2;
     TDirectoryPtr fdir = fs.find_directory(directoryName);
     if (fdir == nullptr)
         return 3;
     if(std::string(fdir->get_name()) != dir->get_name())
     //if (fdir.get_offset() != dir.get_offset())
         return 4;
     fs.remove_directory(directoryName);
     TDirectoryPtr fdir2;
     try
     {
         fdir2 = fs.find_directory(directoryName);
     }
     catch (boost::exception& e)
     { 
         if (*boost::get_error_info<errinfo_fs_code>(e) != fs_error::DIRECTORY_NOT_FOUND)
             return 5;
     }
     if (fdir2 != nullptr)
         return 6;
     return 0;
 }

 int TestTopLevelResetStorage()
 {
     const char* dir_name = "./test_dir/";
     {
         FileSystem fs;
         TDirectoryPtr dir = fs.create_directory(dir_name);
         if (dir == nullptr)
             return 1;
         fs.remove_storage();
     }
     {
         FileSystem fs;
         TDirectoryPtr fdir2;
         try
         {
             fdir2 = fs.find_directory(dir_name);
         }
         catch (boost::exception& e)
         {
             if (*boost::get_error_info<errinfo_fs_code>(e) == fs_error::DIRECTORY_NOT_FOUND)
                 return 0;
         }
         if (fdir2 != nullptr)
             return 2;
     }
     return 0;
 }

 int TestTopLevelFileDescription()
 {
     const char* name = "./test_file_description.txt";
     FileSystem fs;
     TFilePtr file = fs.create_file(name);
     FileDescription fdesc = file->get_description();
     FileDescription tdesc = fs.get_file_desc(name);
     if (fdesc.fileID != tdesc.fileID)
         return 1;
     if (std::string(fdesc.name) != tdesc.name)
         return 2;
     if (fdesc.size != tdesc.size)
         return 3;
     fs.remove_file(name);
     return 0;
 }

 int TestMultiThreadAccess()
 {
     return 0;
 }

 int StressTestFileCreateRemove()
 {
     // TODO: temporary swithc off logging to speed up
     // TODO: check memory leaks. create and remove file in cycle. size of storage should not increase
     return 0;
 }

 
int main()
{
    int rc;
    // setup logging filter
    g_level = boost::log::trivial::severity_level::error;

    try
    {
        FileSystem fs;
        fs.remove_storage();
    }
    catch (...) {}

    //TestRemoveCreateFiles();
    //TestPrintFilesInfo(1, 100);
    
    //TestDirectory();
    //TestDirectory2();

    rc = TestHelper();
    if (rc == 0)
        std::cout << "Test TestHelper passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestHelper test. case: " << rc << std::endl;
        assert(0);
    }

    rc = TestCreateDirectoriesTree();
    if (rc == 0)
        std::cout << "Test TestCreateDirectoriesTree passed" << std::endl;
    else
        std::cout << "Error: failed TestCreateDirectoriesTree test. case: " << rc << std::endl;

    rc = TestTopLevelCreateDirectory();
    if (rc == 0)
        std::cout << "Test TestTopLevelCreateDirectory passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestTopLevelCreateDirectory test. case: " << rc << std::endl;
        assert(0);
    }

    rc = TestTopLevelCreateFile();
    if (rc == 0)
        std::cout << "Test TestTopLevelCreateFile passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestTopLevelCreateFile test. case: " << rc << std::endl;
        assert(0);
    }

    rc = TestTopLevelRemoveDirectories();
    if (rc == 0)
        std::cout << "Test TestTopLevelRemoveDirectories passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestTopLevelRemoveDirectories test. case: " << rc << std::endl;
        assert(0);
    }

    rc = TestTopLevelResetStorage();
    if (rc == 0)
        std::cout << "Test TestTopLevelResetStorage passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestTopLevelResetStorage test. case: " << rc << std::endl;
        assert(0);
    }

    g_level = boost::log::trivial::severity_level::error;
    rc = TestTopLevelFileDescription();
    if (rc == 0)
        std::cout << "Test TestTopLevelFileDescription passed" << std::endl;
    else
    {
        std::cout << "Error: failed TestTopLevelFileDescription test. case: " << rc << std::endl;
        assert(0);
    }
}
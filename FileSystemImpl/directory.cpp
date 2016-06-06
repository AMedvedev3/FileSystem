/**
@file
Directory class for FileSystem implementation
@author Alexey Medvedev
*/

#include "helper.h"
#include "directory.h"
#include "file_system.h"
#include "exception.h"
#include "log.h"

const char* ROOT_DIRECTORY_NAME = ".";

Directory::Directory(const char* name, TDirectoryPtr parent, FileSystem& fs)
{
    try
    {
        this->parent = parent;
        this->description = fs.add_file_desc(name, NULL, 0);
        this->description->type = DescriptionType::DirectoryType;

        this->sub_directories = fs.m_seg->construct<TDirectorySet>(noname)
            (
                std::less<shared_string>(),
                TDirectorySet::allocator_type(fs.m_seg->get_segment_manager())
                );

        this->files = fs.m_seg->construct<TFileSet>(noname)
            (
                std::less<shared_string>(),
                TFileSet::allocator_type(fs.m_seg->get_segment_manager())
                );
    }
    catch (...)
    {
        // TODO: move to clean() function
        if (this->description != nullptr)
        {
            fs.m_file_descriptions->erase(this->description->fileID);
            this->description = nullptr;
        }

        if (this->sub_directories != nullptr)
        {
            fs.m_seg->destroy_ptr<TDirectorySet>(this->sub_directories.get());
            this->sub_directories = nullptr;
        }

        if (this->files != nullptr)
        {
            fs.m_seg->destroy_ptr<TFileSet>(this->files.get());
            this->files = nullptr;
        }
         
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Error in Directory instance construction."));
        
    }
}


TDirectoryPtr Directory::construct(const char* name, TDirectoryPtr parent,
    FileSystem& fs, int l)
{
    if (l > 1)
    {
        LF << "Unrecoverable Error: not enough memory to create directory." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Unrecoverable Error: not enough memory to create directory."));
    }

    TDirectoryPtr new_dir = NULL;
    try
    {
        new_dir = fs.m_seg->construct<Directory>(noname) (name, parent, fs);
    }
    catch (bip::bad_alloc&)
    {
        fs.grow_storage();
        l++;
        new_dir = construct(name, parent, fs, l);
        --l;
    }
    return new_dir;
}


void Directory::remove_directory(const shared_string& name)
{
    assert(sub_directories);
    //assert(m_files);
    auto it = sub_directories->find(name);
    if (it != sub_directories->end())
    {
        Directory& dir = it->second;
        assert(dir.sub_directories);
        // check that sub directories and files set are empty
        if (dir.sub_directories->empty())
        {
            sub_directories->erase(it);
            LF << "Direcory \"" << name << "\" successfully deleted" ;
        }
        else
        {
            LF << "Error: Direcory \"" << name << "\" is not empty" ;
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::DIRECTORY_IS_NOT_EMPTY)
                << errinfo_message("Error in removing directory. Directory is not empty."));
        }
    }
}


TDirectoryPtr Directory::add_directory(FileSystem& fs, const char* name)
{
    assert(name);
    assert(fs.m_seg);

    TDirectoryPtr new_dir = Directory::construct(name, this, fs);
    LF << "New dir \"" << new_dir->get_name() << "\"" ;

    auto rc = sub_directories->insert(std::make_pair(mkstr(name, fs), *new_dir));
    if (!rc.second)
    {
        LF << "Directory \"" << name << "\" already exists in directory \"" << get_name() ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::DIRECTORY_ALREADY_EXIST)
            << errinfo_message("Error in adding directory. Directory already exists."));
    }
    return new_dir;
}
 

std::string Directory::get_path() const
{
    std::string res = get_name();
    res.append("/");
    auto p = parent;
    while (p)
    {
        res = std::string(p->get_name()).append("/").append(res);
        p = p->parent;
    }
    res.insert(0, "/");
    return res;
}


TDirectoryPtr Directory::find_directory(FileSystem& fs, TDirectoryPtr& root, const char* path) 
{
    assert(root);
    assert(path);

    ParsedPath p = ParsedPath::parse_dir_path(path);
    if (p.directories.empty())
    {
        LF << "Error: invalid directory path" ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::INVALID_DIRECTORY_NAME)
            << errinfo_message("Cannot find directory. Invalid directory path format."));
    }
    
    TDirectoryPtr dir = root;
    string cur_path("/");
        
    auto it = p.directories.begin();
    auto end = p.directories.end();
    for (; it != end; ++it)
    {
        if (it == p.directories.begin() && *it == root->get_name())
            continue;

        cur_path.append(*it).append("/");
        auto pair = dir->sub_directories->find( mkstr((*it).c_str(), fs) );
        if (pair != dir->sub_directories->end())
        {
            dir = &pair->second;
        }
        else
        {
            LF << "Error: path not found \"" << cur_path << "\"" ;
            LF << "Sub directories list: ";
            for (auto it1 = dir->sub_directories->begin(); it1 != dir->sub_directories->end(); it1++)
            {
                LF << it1->second.get_name() << ";" ;
            }
            LF ;
            return nullptr;
        }
    }

    return dir;
}

TDirectoryPtr Directory::find_sub_directory(const shared_string& name) const
{
    auto it = this->sub_directories->find(name);
    if (it != this->sub_directories->end())
        return &it->second;
    else
        return nullptr;
}

bip::offset_ptr<File> Directory::find_file(const shared_string& name)
{
    auto it = files->find(name);
    if (it != files->end())
        return &it->second;
    else
        return NULL;
}


void Directory::add_file(const shared_string& name, File& file)
{
    files->insert(std::make_pair(name, file));
}


void Directory::remove_file(const shared_string& name)
{
    files->erase(name);
}


void Directory::rename(const char* new_name)
{
    description->set_name(new_name);
}

bool Directory::empty()
{
    return this->sub_directories->empty() &&
        this->files->empty();
}

void Directory::clean(FileSystem& fs)
{
    TFileID id = 0;
    if (this->description != nullptr)
    {
        id = this->description->fileID;
        fs.m_file_descriptions->erase(this->description->fileID);
        this->description = nullptr;
    }

    if (this->sub_directories != nullptr)
    {
        fs.m_seg->destroy_ptr<TDirectorySet>(this->sub_directories.get());
        this->sub_directories = nullptr;
    }

    if (this->files != nullptr)
        this->files = nullptr;

    if(id != 0)
        fs.m_file_id_manager->reg_deleted_file(id);
}


FileDescription Directory::get_description()
{
    return *this->description.get();
}
#include "helper.h"
#include "directory.h"
#include "file_system.h"

const char* ROOT_DIRECTORY_NAME = ".";

Directory::Directory(const char* name, TDirectoryPtr parent, FileSystem& fs)
{
    try
    {
        this->parent = parent;
        this->description = fs.add_file_desc(name, NULL, 0);
        // TODO: check success
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
        //TODO: do that in the destructor?
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
         
        // TODO: make standard exception!!!
  
        throw;
    }
}


TDirectoryPtr Directory::construct(const char* name, TDirectoryPtr parent,
    FileSystem& fs, int l)
{
    if (l > 1)
    {
        std::cout << "Unrecoverable Error: not enough memory to create directory." << std::endl;
        assert(0);
        // TODO: throw exception
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
        Directory& dir = it->second; // ??
        assert(dir.sub_directories);
        //// check that sub directories and files set are empty
        if (dir.sub_directories->empty())
        {
            sub_directories->erase(it);
            // TODO: replace by logging
            std::cout << "Direcory \"" << name << "\" successfully deleted" << std::endl;
        }
        else
        {
            // TODO: replace by exception
            std::cout << "Error: Direcory \"" << name << "\" is not empty" << std::endl;
        }
    }
}


TDirectoryPtr Directory::add_directory(FileSystem& fs, const char* name)
{
    assert(name);
    assert(fs.m_seg);

    TDirectoryPtr new_dir = Directory::construct(name, this, fs);
    std::cout << "New dir \"" << new_dir->get_name() << "\"" << std::endl;

    auto rc = sub_directories->insert(std::make_pair(mkstr(name, fs), *new_dir));
    if (!rc.second)
    {
        // TODO: directory already exists exception
        std::cout << "Directory \"" << name << "\" already exists in directory \"" << get_name() << std::endl;
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
        std::cout << "Error: invalid directory path" << std::endl;
        assert(0);
        return nullptr;
    }
    
    TDirectoryPtr dir = root;
    string cur_path("/");
        
    auto it = p.directories.begin();
    auto end = p.directories.end();
    for (; it != end; ++it)
    {
        if (it == p.directories.begin() && *it == root->get_name())
            continue;

        // TODO: TEMP test only
        auto sz = dir->sub_directories->size();


        cur_path.append(*it).append("/");
        auto pair = dir->sub_directories->find( mkstr((*it).c_str(), fs) );
        if (pair != dir->sub_directories->end())
        {
            dir = &pair->second;
        }
        else
        {
            std::cout << "Error: path not found \"" << cur_path << "\"" << std::endl;
            std::cout << "Sub directories list: ";
            for (auto it1 = dir->sub_directories->begin(); it1 != dir->sub_directories->end(); it1++)
            {
                std::cout << it1->second.get_name() << ";" ;
            }
            std::cout << std::endl;
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
        //fs.m_seg->destroy_ptr<TFileSet>(this->files.get());
        this->files = nullptr;
    }
}


FileDescription Directory::get_description()
{
    return *this->description.get();
}
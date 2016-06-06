/**
@file
File class for FileSystem implementation
@author Alexey Medvedev
*/

#include "common.h"
#include "helper.h"
#include "file.h"
#include "directory.h"
#include "exception.h"
#include "log.h"

File::File(TFileID id, const char* name, TDirectoryPtr& dir, FileSystem& fs)
{
    try
    {
        this->description = fs.add_file_desc(name, NULL, 0);
        if (this->description == NULL)
        {
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
                << errinfo_message("Cannot construct File description instance for file."));
        }

        this->description->type = DescriptionType::FileType;
        this->id = this->description->fileID;

        auto res = fs.m_files_data->insert(
            std::make_pair(this->id, 
            TFileData(TFileData::allocator_type(fs.m_seg->get_segment_manager()))));

        assert(res.second);
        this->file_data = &res.first->second;
        if (this->file_data == nullptr)
        {
            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
                << errinfo_message("Cannot construct File data instance."));
        }

        // add file to parent directory
        this->directory = dir;
        dir->add_file(mkstr(name, fs), *this);

    }
    catch (...)
    {
        // TODO: move to clean() function
        if (this->id != 0)
        {
            if (this->description != nullptr)
            {
                fs.m_file_descriptions->erase(this->id);
                this->description = nullptr;
            }
            dir->remove_file(mkstr(description->name, fs));

            if (this->file_data != nullptr)
            {
                fs.m_files_data->erase(this->id);
                this->file_data = nullptr;
            }

            fs.m_file_id_manager->reg_deleted_file(id);
            this->id = 0;

            BOOST_THROW_EXCEPTION(common_exception()
                << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
                << errinfo_message("Cannot construct File instance."));
        }
    }
}


bip::offset_ptr<File> File::construct(const char* name, TFileID id, 
    TDirectoryPtr& dir, FileSystem& fs, int l)
{
    if (l > 1)
    {
        LF << "Unrecoverable Error: not enough memory to create file." ;
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Unrecoverable error. Not enough memory to construct file."));
    }

    bip::offset_ptr<File> file = NULL;
    try
    {
        file = fs.m_seg->construct<File>(noname) (id, name, dir, fs);
    }
    catch (bip::bad_alloc&)
    {
        fs.grow_storage();
        l++;
        file = construct(name, id, dir, fs, l);
        --l;
    }
    return file;
}

void File::rename(const char* new_name) 
{ 
    description->set_name(new_name); 
}

size_t File::get_size() 
{ 
    return file_data->size(); 
}

std::string File::get_name() 
{ 
    return description->name; 
}

std::string File::get_path() 
{ 
    return directory->get_path() + get_name(); 
}

TDirectoryPtr File::get_directory()
{
    return directory;
}

TFileID File::get_id()
{
    return description->fileID;
}

// @buffer - buffer to write
// @cb - size of buffer, number of bytes to write
// @offset - start position in file where to start writing new data
// @written - return value - returns number of really written bytes to file
// @fs - reference to fs for inner purposes
// @l - reentrant counter used for exception handling. Don't set it explicitely, always use default value  
// returns last position in file after a write finished and ref to count of written bytes
size_t File::write(const char* buffer, size_t cb, size_t offset, size_t* written, FileSystem& fs, int l)
{
    assert(this->file_data != nullptr);

    if (l > 1)
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::STORAGE_STRUCTURE_ERROR)
            << errinfo_message("Unrecoverable bad allocation error."));
    }
    
    // check file constraints
    if (offset > file_data->size())
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::GENERAL_WRITE_FAULT)
            << errinfo_message("Start writing position after end of file."));
    }

    // the fs grows storage by 200MB portions so it has sense 
    // to reserve maximum 200 MB for file data and 
    // writes only part of source buffer if the buffer size exceeds 200MB
    const size_t c190MB = 190'1000'1000;
    cb = cb > c190MB ? c190MB : cb;
    size_t last_pos;
    auto sb = file_data->size();
    try
    {
        // grow the file if necessary
        if (offset + cb > file_data->size())
        {
            file_data->resize(offset + cb);
            description->size = offset + cb;
        }
    }
    catch (bip::bad_alloc&)
    {
        l++;
        last_pos = write(buffer, cb, offset, written, fs, l);
        l--;
        fs.grow_storage();
    }

    last_pos = offset + cb;
    // all required memory is reserved. there should be no bad_alloc further
    std::copy(buffer, buffer + cb - 1, file_data->begin() + offset);
          
    auto sz = file_data->size();
    assert(file_data->size() >= cb + offset);
    *written = cb;

    return last_pos;
}

size_t File::read(char* buffer, size_t cb, size_t offset, size_t* readed)
{
    assert(readed);

    // TODO: check parameters, and throw execption

    if (offset >= file_data->size())
    {
        BOOST_THROW_EXCEPTION(common_exception()
            << errinfo_fs_code(fs_error::GENERAL_READ_FAULT)
            << errinfo_message("Start reading position after end of file."));
    }
    
    size_t size = file_data->size();
    cb = size - offset < cb ? size - offset : cb;

    size_t last_pos = offset + cb;
    std::copy(file_data->begin() + offset, 
        file_data->begin() + last_pos - 1, 
        buffer);

    *readed = cb;

    return last_pos;
}

void File::clean(FileSystem& fs)
{
    if (this->id != 0)
    {
        shared_string name = mkstr("", fs);
        if (this->description != nullptr)
        {
            name = mkstr((const char*)description->name, fs);
            fs.m_file_descriptions->erase(this->id);
            this->description = nullptr;
        }

        if (this->file_data != nullptr)
        {
            fs.m_files_data->erase(this->id);
            this->file_data = nullptr;
        }

        if (this->directory != nullptr && !name.empty())
        {
            this->directory->remove_file(name);
            this->directory = nullptr;
        }

        fs.m_file_id_manager->reg_deleted_file(id);
        this->id = 0;
    }
}

FileDescription File::get_description()
{ 
    return *this->description.get(); 
}
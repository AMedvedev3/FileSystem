#pragma once
/**
@file
FileIDManager class definition for FileSystem
@author Alexey Medvedev
*/
#include "common.h"

using TFreeFileIds = shared_list<TFileID>;

// FileIDManager tracks usage of File Ids.
// It keeps list of freed ID's released by removed files.
class FileIDManager
{
public:

    // Request free file id from FieldIDManager
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

    // Register id of removed file or directory
    void reg_deleted_file(TFileID id)
    {
        // check that removed last file in the list then just decrease m_last_free_id
        if (id != m_last_free_id - 1)
            m_free_ids->push_back(id);
        else
            m_last_free_id--;
    }

    // Factory for construct the FileIDManager in FileSystem
    static FileIDManager* load_or_create(const std::shared_ptr<bip::managed_mapped_file>& seg)
    {
        FileIDManager* file_id_manager = seg->find_or_construct <FileIDManager>("FileIDManager")();

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

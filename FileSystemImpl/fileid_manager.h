#pragma once
#include "common.h"

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

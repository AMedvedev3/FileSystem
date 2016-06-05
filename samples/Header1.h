#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>

#include <iostream>
#include <chrono>

class ScopedTM
{
    std::string _caption;
    std::chrono::high_resolution_clock::time_point _start;
    int _count;
public:
    ScopedTM(std::string caption) : _caption(caption), _count(-1) {
        _start = std::chrono::high_resolution_clock::now();
        std::cout << std::endl << _caption << " ------------ " << std::endl;
    }

    void setCount(int  cnt) {
        _count = cnt;
    }

    ~ScopedTM() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - _start;
        std::cout << std::endl << "took : " << diff.count() << " seconds" << std::endl;
        if (_count != -1) {
            std::cout << "made : " << _count << " records, " << std::endl
                << "perf : " << _count / diff.count() << " recs/sec " << std::endl;
        }
    }
};

namespace MappedObjects {

    //
    // boost interprocess helpers
    // + prototype based inheritance as virtuality is forbidden 
    //    and voi-la!
    //
    namespace bi = boost::interprocess;

    //
    // allocators 
    //    - static (templated)
    //
    class StoreSM
    {
    public:
        bi::managed_mapped_file::segment_manager* _psm;

        static StoreSM* instance() {
            static StoreSM* _this = new StoreSM();
            return _this;
        }
    };

    template <class T > class AllocatorSM : public bi::allocator<T, bi::managed_mapped_file::segment_manager> {
    public:
        AllocatorSM() : bi::allocator<T, bi::managed_mapped_file::segment_manager>(StoreSM::instance()->_psm) {}
    };

    //
    // basic types
    //
    typedef bi::basic_string <char, std::char_traits<char>, AllocatorSM<char> >  String;

    template <class T> class Vector : public bi::vector < T, AllocatorSM<T> > {};


    //
    // object model
    //
    class SubEntry
    {
    public:
        int _id;
        String _name;
    };

    class RootObject2
    {
    public:
        int _id;
        String _name2;
        bi::offset_ptr<RootObject2> _other;
        Vector<SubEntry> _docs;
    };
}#pragma once

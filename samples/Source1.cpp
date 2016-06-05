#include <iostream>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/flat_map.hpp>


int test(void)
{
    namespace ipc = boost::interprocess;

    ipc::managed_mapped_file mfile(ipc::open_or_create, "hoge.dat", 65536);

    // int vector
    using int_allocator_type = ipc::allocator<int, ipc::managed_mapped_file::segment_manager>;
    using int_vector_type = ipc::vector<int, int_allocator_type>;
    int_vector_type & mvec = *mfile.find_or_construct<int_vector_type>("vector")(mfile.get_segment_manager());

    mvec.push_back(1);
    std::cout << "mvec size = " << mvec.size() << std::endl;


    // string
    using char_allocator_type = ipc::allocator<char, ipc::managed_mapped_file::segment_manager>;
    using string_type = ipc::basic_string<char, std::char_traits<char>, char_allocator_type>;
    string_type & mstr = *mfile.find_or_construct<string_type>("string")(mfile.get_segment_manager());

    mstr += "1";
    std::cout << "mstr = " << mstr << std::endl;


    // string vector
    using string_allocator_type = ipc::allocator<string_type, ipc::managed_mapped_file::segment_manager>;
    using string_vector_type = ipc::vector<string_type, string_allocator_type>;
    string_vector_type & msvec = *mfile.find_or_construct<string_vector_type>("strvec")(mfile.get_segment_manager());

    string_type tstr("hoge", mfile.get_segment_manager());
    msvec.push_back(tstr);
    for (int i = 0; i < msvec.size(); i++) {
        std::cout << "msvec.at(" << i << ") = " << msvec[i] << std::endl;
    }

    // string map
    using dictionary_allocator_type = ipc::allocator<std::pair<const string_type, string_type>, ipc::managed_mapped_file::segment_manager>;
    using dictionary_type = ipc::flat_map<string_type, string_type, std::less<string_type>, dictionary_allocator_type>;
    dictionary_type & msmap = *mfile.find_or_construct<dictionary_type>("strmap")(std::less<string_type>(), mfile.get_segment_manager());

    string_type key("hoge", mfile.get_segment_manager());
    msmap.insert(std::make_pair(key, string_type("hoge", mfile.get_segment_manager())));
    std::cout << msmap.find(key)->second << std::endl;
}
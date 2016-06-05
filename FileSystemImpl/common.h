#pragma once

#include <Windows.h>

#include <algorithm>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/map.hpp>

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
#include <boost/interprocess/containers/stable_vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
//standard defs should be defined after interp
#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
namespace bip = boost::interprocess;

// unique id, using for check file system consistency
using TFileID = uintmax_t;
template <typename T> using shared_alloc = bip::allocator<T, bip::managed_mapped_file::segment_manager>;
template <typename T> using shared_vector = boost::container::vector< T, shared_alloc<T> >;
template <typename T> using shared_set = boost::container::set<T, std::less<T>, shared_alloc<T> >;
template <typename T> using shared_list = boost::container::list<T, shared_alloc<T> >;
using shared_string = bip::basic_string<char, std::char_traits<char>, shared_alloc<char> >;

class Directory;
class FileSystem;
struct FileDescription;

template <typename K, typename V, typename Cmp = std::less<K>, typename P = std::pair<const K, V > >
using shared_map = bip::map<K, V, Cmp, shared_alloc<P> > ;

using TFileDescriptionSet = shared_map<TFileID, FileDescription>;
using TDirectorySet = shared_map< shared_string, Directory >;
using TFileData = shared_vector<char>;
using TFileDataSet = shared_map< TFileID, shared_vector<char> >;

using TDirectoryPtr = bip::offset_ptr<Directory>;
using TFileDataPtr = bip::offset_ptr< shared_vector<char> >;

using TStorage = bip::managed_mapped_file;
using TStoragePtr = std::shared_ptr<TStorage>;

#define noname (bip::anonymous_instance)




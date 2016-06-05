////////////////////////////////////////////////////
//
// Args:
//    me - this node name
//    config file name - optional, default rl.config.json
//
// Functionality:
//       web ui
//       writer
//       reader
//       subscribe 
//       bridge
//
////////////////////////////////////////////////////

#include <boost/thread.hpp>
#include "storage.h"
#include "client_api.h"
#include "cluster_api.h"
#include "web_admin.h"
#include "object_store.h"

void testRead();
void testWrite();
void testStripe();


int main(int argc, char *argv[]) {
    std::cout << "RethinkLog - the revolution (version 100500)" << std::endl;

    testWrite(); // build file larger than memory
                 //   testRead();
                 //
    char a;
    std::cin >> a;
    if (a != 'c')
        return 0;

    Config cfg(argc, argv);
    {
        //
        // task queue
        //
        boost::asio::io_service io;

        // Ctrl^C and kill
        boost::asio::signal_set sigs(io, SIGINT, SIGTERM);
        sigs.async_wait([&](const boost::system::error_code& error, int signal_number) {
            if (!error) {
                std::cout
                    << "Ctrl^C invoked." << std::endl
                    << "Shutting down the system." << std::endl;
                io.stop();
            }
        });

        //
        // services
        //

        //
        //
        // working name - CloudPart.com
        //
        // log writer (-- must be in single executable (maybe as library)
        //             with state due to performance of operations' reasons)
        // log reader
        //
        // state keeper
        //
        // tasks' execution
        //
        // admin itf
        // rest data itf
        // rest log itf

        ///
        /// TODO - plugins
        ///
        // log
        //    triggers
        //    listeners
        //
        // state
        //    object model + serializer + partitioning algo
        //
        //    minimal tasks (object methods)
        //
        // llvm executor
        //       + deployment infrastrucutre
        // python executor
        // java executor
        // .NET executor
        //


        Storage			   stg(io, cfg);
        ClientAPISvc		wrt(io, cfg, stg);
        WebSvc            web(io, cfg);

        //
        // events' processing
        //
        boost::thread_group tg;
        for (int i = 0; i < boost::thread::hardware_concurrency(); ++i) {
            tg.create_thread([&io]() { io.run(); });
        }
        tg.join_all();
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////
//
// tests' bodies
//
////////////////////////////////////////////////////////////////////////////
void testWrite() {
    namespace mo = MappedObjects;
    namespace bi = boost::interprocess;
    {
        ScopedTM tm("write");
        std::unique_ptr<bi::managed_mapped_file> mapped_file;

        mapped_file.reset(new bi::managed_mapped_file(bi::open_or_create, "data.bin",  // open memmapped file
            (uint64_t)64 * (uint64_t)1024 * (uint64_t)1024));               // 64 Mb default
        mo::StoreSM::instance()->_psm = mapped_file->get_segment_manager();               // setup static allocators

        mo::RootObject2* pp = nullptr;
        int x;
        for (x = 1; x < 50000000; ++x) { // 50 M records
            std::stringstream obj_name;
            obj_name << "obj" << x;
            if (x % 10000 == 0) std::cout << ".";

            try {
                mo::RootObject2* p;
                p = mapped_file->find_or_construct<mo::RootObject2>(obj_name.str().c_str())();
                p->_id = x;
                p->_name2 = "object of its kind";
                p->_other = pp;

                p->_docs.clear();

                p->_docs.push_back(mo::SubEntry{ 1, obj_name.str().c_str() });
                p->_docs.push_back(mo::SubEntry{ 2, obj_name.str().c_str() });
                p->_docs.push_back(mo::SubEntry{ 3, obj_name.str().c_str() });

                p->_docs.pop_back();

                pp = p;
            }
            catch (bi::bad_alloc& e) {
                ScopedTM tmr("bad alloc caught, resizing"); // русский текст
                mapped_file.reset(nullptr);
                bi::managed_mapped_file::grow("data.bin", (uint64_t)64 * (uint64_t)1024 * (uint64_t)1024);

                mapped_file.reset(new bi::managed_mapped_file(bi::open_or_create, "data.bin",                          // open memmapped file
                    (uint64_t)64 * (uint64_t)1024 * (uint64_t)1024));   // 64 Mb default
                mo::StoreSM::instance()->_psm = mapped_file->get_segment_manager();                                       // setup static allocators
            }
        }
        tm.setCount(x);
    }
    {
        ScopedTM tm("shrink");
        boost::interprocess::managed_mapped_file::shrink_to_fit("data.bin");
    }
}

void testRead() {
    ScopedTM tm("read");

    boost::interprocess::managed_mapped_file fl(boost::interprocess::open_only, "data.bin");
    int cnt = 0;
    std::cout << "named objects ------------- {";
    for (auto it = fl.named_begin(); it != fl.named_end(); ++it) {
        //std::cout << std::endl << it->name();
        MappedObjects::RootObject2* p = (MappedObjects::RootObject2*) it->value();
        if (p->_name2 == "n/a")              // pseudo 'find'
            std::cout << std::endl << it->name();
        //std::cout << "; id = " << p->_id << "; name2 = " << p->_name2 << "; docs count = " << p->_docs.size();
        //if ( p->_other ) std::cout << "; other id = " << p->_other->_id;                                // reference to other object
        //std::cout << "; docs : "; for ( auto d : p->_docs )  std::cout << d._name << ",";               // sub entry names
        ++cnt;
        if (cnt % 1000000 == 0)
            std::cout << ".";
    }
    tm.setCount(cnt);
    std::cout << std::endl << "}";
}

void testStripe() {
    /// tests

    Config cfg(1, 0);

    std::cout << "=== tests mode ===" << std::endl;
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto st = Stripe::writer("boo-ga-ga-000000111111", cfg);
        int x = 0;
        for (; x < 10 * 1000 * 1000; ++x) {
            st(UINT64_MAX, "dodood", "jshgjhslk\r\njghsdlkjfhglksdfhglkj\r\nfdshglkdshjfhsljhflksjhfdlj\r\nkshdflkjhsdljfhlkjdshgsgfdfgri"
                "u3fhihfkldshlkfjhdslf\"\"\"\" sadasdsadsadasdsadsadasdad{}{} jhkjfhsdkjhfkjsdhkfhh}sdfdsfdsfdsfs}} "
                "u3fhihfkldshlkfjhdslf\"\"\"\" sadasdsadsadasdsadsadasdad{}{} jhkjfhsdkjhfkjsdhkfhh}sdfdsfdsfdsfs}} "
                "u3fhihfkldshlkfjhdslf\"\"\"\" sadasdsadsadasdsadsadasdad{}{} jhkjfhsdkjhfkjsdhkfhh}sdfdsfdsfdsfs}} "
                "u3fhihfkldshlkfjhdslf\"\"\"\" sadasdsadsadasdsadsadasdad{}{} jhkjfhsdkjhfkjsdhkfhh}sdfdsfdsfdsfs}} "
                "jshgjhslk\r\njghsdlkjfhglksdfhglkj\r\nfdshglkdshjfhsljhflksjhfdlj\r\nkshdflkjhsdljfhlkjdshgsgfdfgri", "");
            if (x % 50000 == 0)
                std::cout << ".";
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << std::endl
            << "made : " << x << " records, " << std::endl
            << "took : " << diff.count() << " seconds" << std::endl
            << "perf : " << x / diff.count() << " recs/sec " << std::endl;
    }
}
/*  shared_string str = *m_seg->construct<shared_string>(bip::anonymous_instance)
(name, shared_string::allocator_type(m_seg->get_segment_manager()));*/
//m_sub_directories->insert(std::make_pair(shared_string("test", m_seg->get_segment_manager()), 
//    new_directory));
//auto pair = std::make_pair(shared_string("test", m_seg->get_segment_manager()), new_directory);
//shared_string str = shared_string("test", m_seg->get_segment_manager());
//using TStrDescription = shared_map<shared_string, FileDescription>;
//TStrDescription* map = m_seg->construct<TStrDescription>(bip::anonymous_instance)
//    (
//        std::less<shared_string>(),
//        TStrDescription::allocator_type(m_seg->get_segment_manager())
//    );

//FileDescription desc("test", "text");
//map->insert(std::make_pair(shared_string("test", m_seg->get_segment_manager()),
//    desc));
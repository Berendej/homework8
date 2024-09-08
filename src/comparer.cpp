#include "comparer.h"
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/bind/bind.hpp> 
using std::cout;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#define DEFAULT_BLOCK_SIZE 1024

comparer_c::comparer_c() : 
    m_recursive(false)
    , m_block_size(1024)
    , m_min_size(1)
    , m_algo_md5(true)
{
}

/*
• директории для сканирования (может быть несколько)
• директории для исключения из сканирования (может быть несколько)
• уровень сканирования (один на все директории, 0 - только указанная
директория без вложенных)
• минимальный размер файла, по умолчанию проверяются все файлы
*** больше *** 1 байта.
• маски имен файлов разрешенных для сравнения (не зависят от
регистра)
• размер блока, которым производится чтения файлов, в задании этот
размер упоминается как S
• один из имеющихся алгоритмов хэширования (crc32, md5 -
конкретные варианты определить самостоятельно), в задании
эта функция упоминается как H
*/
void comparer_c::set_dirs(string_vector_t dirs)
{
    std::move(dirs.begin(), dirs.end(), std::back_inserter(m_dirs)); 
}

void comparer_c::set_excludes(string_vector_t dirs)
{
    std::move(dirs.begin(), dirs.end(), 
        std::inserter(m_excludes, m_excludes.begin()));
}

void comparer_c::set_recursive(int r)
{
    m_recursive = (r != 0);
}

void comparer_c::set_minsize(unsigned sz)
{
    m_min_size = sz;
}

void comparer_c::set_mask(const std::string msk)
{
    m_mask = msk; // used as a flag 
    m_regex_mask.assign(msk);
}

void comparer_c::set_block_size(unsigned int bs)
{
    m_block_size = bs;
}

void comparer_c::set_algo(unsigned a)
{
    if ( 0 == a )
    {
        m_algo_md5 = true;
    }
    else
    {
        m_algo_md5 = false;
    }
}

bool comparer_c::open_directories()
{
    if ( 0 == m_dirs.size() )
    {
        m_dirs.push_back(".");
    }
    for( auto d : m_dirs )
    {
        get_dir_files(d);
    }
    return true;
}

bool comparer_c::file_name_suitable( const std::string &fn)
{
    if ( 0 == m_mask.length() )
    {
        return true;
    }
    boost::smatch what;
    return boost::regex_match( fn, what, m_regex_mask );
}

void comparer_c::get_dir_files(const std::string &d)
{
    fs::path dir = d;
    fs::directory_iterator it(dir), end;
    for (auto& entry : boost::make_iterator_range(it, end))
    {
        if (is_regular_file(entry))
        {
            // entry.path() replaced with abs
            fs::path abs = fs::canonical(entry.path());
            // at first filter with min size filter ( much simpler and faster 
            // than filename template filter )
            size_t sz = fs::file_size(abs.native());
            if ( sz >= m_min_size )
            {
                if ( file_name_suitable(abs.native()) )
                {
                    sp_phile_t sp_phile(new phile_c(abs.native(), 
                                             sz, m_block_size, m_algo_md5));
                    m_size_map.insert(  std::pair<unsigned int, 
                            sp_phile_t>(sz, sp_phile));
                }
            }
        }
        else if ( m_recursive && is_directory(entry) )
        {
            boost::filesystem::path p(entry.path().native());
            if ( not excluded(p.filename().native()) )
            {   
                get_dir_files(entry.path().native());
            }
        }
    }
}

bool comparer_c::excluded(const std::string &d)
{
    bool result = (m_excludes.find(d) != m_excludes.end() );
    return result;
}

bool comparer_c::get_options(int argc, char *argv[])
{
    bool result = true;
    try {
        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "This screen")
                ("dir,d", po::value<string_vector_t>()->
                    notifier(boost::bind(&comparer_c::set_dirs, this, boost::placeholders::_1)),
                    "directories to scan (several directories allowed) current dir by default")
                ("excl,e", po::value<string_vector_t>()->
                    notifier(boost::bind(&comparer_c::set_excludes, this, boost::placeholders::_1)),
                    "directories to EXCLUDE from scan (several directories allowed)")
                ("recur,r", po::value<int>()->default_value(0)->
                    notifier(boost::bind(&comparer_c::set_recursive, this, boost::placeholders::_1)),
                    "scan depth 0-flat , 1-recursively default 0")
                ("size,s", po::value<unsigned>()->default_value(1)->
                    notifier(boost::bind(&comparer_c::set_minsize, this, boost::placeholders::_1)),
                    "minimal file size 1 byte by default")
                ("mask,m", po::value<std::string>()->default_value("")->
                    notifier(boost::bind(&comparer_c::set_mask, this, boost::placeholders::_1)),
                    "file name mask")
                ("blocksize,b", po::value<unsigned int>()->default_value(DEFAULT_BLOCK_SIZE)->
                    notifier(boost::bind(&comparer_c::set_block_size, this, boost::placeholders::_1)),
                    "block size, 1024 bytes by default")
                ("algo,a", po::value<int>()->default_value(0)->
                    notifier(boost::bind(&comparer_c::set_algo, this, boost::placeholders::_1)),
                    "hash algorithm, 0-md5 , 1-crc32");
        po::variables_map vm;
        po::store(parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help"))
            std::cout << desc << '\n';
    }
    catch (const std::exception &e) {
        std::cerr << "Parameter error: " << e.what() << std::endl;
        result = false;
    }
    return result;
}

void comparer_c::extract_twins(hash_phile_map_t &hh_map)
{
    // start to eliminate hh_map tour by tour
    i_hash_phile_map_t ih = hh_map.begin();
    while( ih != hh_map.end() )
    {
        digest_key_c k = ih->first;
        size_t key_count = hh_map.count(k);
        if ( 1 == key_count )
        {
            // current phile has no twins 
            // it won't go to the next tour of our game
            // thank you phile and goodbye
            hh_map.erase(ih);
        }
        else //  key_count > 1
        {   
            std::pair<i_hash_phile_map_t, i_hash_phile_map_t>
                    ih_pair = hh_map.equal_range(k);
            // several philes has the same hash of current block
            // no more blocks ?
            if ( ih->second->no_more_blocks())
            {
                string_set_t sset;
                // since all philes in hh_map have the
                // same size and identical hash for current (last) block
                // ( and all previous blocks)- they are identical
                std::for_each( ih_pair.first, ih_pair.second,
                            [&](auto par) 
                            {
                                sset.insert(par.second->get_name()); 
                            });
                m_twins.push_back(sset);
                hh_map.erase(ih_pair.first, ih_pair.second);
            }
            else
            {
                // recursion may cause stack overflow
                // in the case when phile size is big and block size is small
                // the recursion depth would be too large (= size/block_size)
                // so make it iterative by using hh_map again
                phile_vector_t tmp_vector;
                std::for_each( ih_pair.first, ih_pair.second,
                    [&](auto par) 
                    {
                        par.second->read_block();
                        tmp_vector.push_back(par.second);
                    });
                // put philes in hh_map with other keys
                hh_map.erase(ih_pair.first, ih_pair.second);
                for( auto sp_ph : tmp_vector )
                {
                    hh_map.insert(std::make_pair(sp_ph->get_block_hash(), sp_ph));
                }
            }
        }
        // start new tour 
        ih = hh_map.begin();
    }
}

void comparer_c::find_twins()
{
    unsigned int k;
    size_t cnt;
    i_uint_phile_map_t it = m_size_map.begin();
    while( it != m_size_map.end() )
    {
        k = it->first;
        cnt = m_size_map.count( k );
        if ( cnt > 1 )
        {
            hash_phile_map_t hh_map;
            i_uint_phile_map_t it2;
            for ( it2 =  m_size_map.equal_range(k).first; 
                  it2 != m_size_map.equal_range(k).second;
                  it2++)
            {
                sp_phile_t sp_phile = it2->second;
                // to avoid comparing same size files "each with each"
                // group them by "block hash" and put in map with block hash 
                // as the key and philes as values.
                sp_phile->read_block();
                hh_map.insert(std::make_pair(sp_phile->get_block_hash(), sp_phile));
            }
            // at this point hh_map groups philes by hash of the first block
            // if identical hashes kept under the same key we must detect 
            // such philes and make further block reading+hashing till they 
            // will be different or till the end of phile (no_more_blocks flag)
            extract_twins(hh_map);
            it = it2; // jump over 
        }
        else
        {
            it++;
        }
    }
}

void comparer_c::show_twins()
{
    for (auto sset : m_twins)
    {
        for( auto s : sset )
        {
            std::cout << s  << std::endl;
        }
        std::cout << std::endl;
    }
}

bool comparer_c::has_twins(const string_vector_t &v)
{
    if ( v.size() < 2 )
    {
        return false;
    }
    int found_entries = 0;
    for( auto sset : m_twins )
    {
        i_string_set_t it = sset.find(v[0]);
        if ( it != sset.end() )
        {
            // if v[0] in twins set< then 
            // ALL others must also present
            for( auto s : v )
            {
                if ( sset.find(s) == sset.end() )
                {
                    // something wrong
                    std::cout << s << " not in results\n";
                    return false;
                }
            }
            return true;
        }
    }
    // nothing found
    return false;
}

#ifndef _COMPARER_H_
#define _COMPARER_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include "phile.h"

using string_vector_t = std::vector<std::string>;
using i_string_vector_t = string_vector_t::iterator;

using string_set_t = std::set<std::string>;
using i_string_set_t = string_set_t::iterator;

using str_vect_set_t = std::vector<string_set_t>;
using i_str_vect_set_t = str_vect_set_t::iterator;

using uint_str_map_t = std::multimap<unsigned int, std::string>;
using i_uint_str_map_t = uint_str_map_t::iterator;

using uint_phile_map_t = std::multimap<unsigned int, sp_phile_t>;
using i_uint_phile_map_t = uint_phile_map_t::iterator;

class comparer_c
{
    bool m_recursive;
    unsigned m_block_size;
    unsigned m_min_size;
    bool m_algo_md5; // true-md5 false-crc32
    std::string m_mask; // if emtpy - no mask
    boost::regex m_regex_mask;

    string_vector_t m_dirs;
    string_set_t m_excludes;
    // m_size_map used for grouping files with the same size 
    uint_phile_map_t m_size_map;


    bool excluded(const std::string &d);
    void get_dir_files(const std::string &d);
    void extract_twins(hash_phile_map_t &hh_map);
    bool file_name_suitable( const std::string &fn);
public:
    // m_twins vector has groups (sets) of identical file names
    str_vect_set_t m_twins;

    comparer_c();
    bool get_options(int argc, char *argv[]);
    bool open_directories();
    void find_twins();
    void show_twins();
    bool has_twins(const string_vector_t &v);

    // public options methods
    void set_dirs(string_vector_t dirs);
    void set_excludes(string_vector_t dirs);
    void set_recursive(int r);
    void set_minsize(unsigned sz);
    void set_mask(const std::string msk);
    void set_block_size(unsigned int bs);
    void set_algo(unsigned a);

};
using sp_comparer_t = boost::shared_ptr<comparer_c>;

#endif
#ifndef _PHILE_H_
#define _PHILE_H_

#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <map>

#include <boost/uuid/detail/md5.hpp>

using uchar_vector_t = std::vector<char>;

struct digest_key_c : public std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
{
    digest_key_c(const boost::uuids::detail::md5::digest_type& d)
    {
        std::get<0>(*this) = d[0];
        std::get<1>(*this) = d[1];
        std::get<2>(*this) = d[2];
        std::get<3>(*this) = d[3];
    }
};

class phile_c
{
    std::string m_name;
    unsigned m_size;
    unsigned m_offset;
    unsigned m_block_size;
    bool m_algo_md5; // true md5, false crc32
    bool m_no_more_blocks;

    boost::uuids::detail::md5::digest_type  m_digest;
    std::ifstream m_ifs;
    uchar_vector_t m_block;
    static int s_open_files;
    void calc_digest();
    void calc_md5();
    void calc_crc32();
public:

    phile_c(std::string name, 
            unsigned size,
            unsigned block_size,
            bool md5_algo);
    virtual ~phile_c();
    const std::string get_name() const;
    const unsigned int get_size() const;
    digest_key_c get_block_hash();
    void read_block();
    bool no_more_blocks()
    {
        return m_no_more_blocks;
    }
};

using sp_phile_t = std::shared_ptr<phile_c>;

using phile_vector_t = std::vector<sp_phile_t>;
using i_phile_vector_t = phile_vector_t::iterator;

using hash_phile_map_t = std::multimap<digest_key_c, sp_phile_t>;
using i_hash_phile_map_t = hash_phile_map_t::iterator;

#endif
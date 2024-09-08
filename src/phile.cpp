#include "phile.h"
#include <iostream>
#include <boost/crc.hpp>

int phile_c::s_open_files = 0;

phile_c::phile_c(std::string name,
                 unsigned size,
                 unsigned block_size,
                 bool md5_algo) :
    m_name(name)
    , m_size(size)
    , m_offset(0)
    , m_block_size(block_size)
    , m_algo_md5(md5_algo)
    , m_no_more_blocks(false)
{
    m_block.resize(block_size);
    if ( !m_algo_md5 )
    {
        // for crc32 3 last bytes of m_digest will always be 0
        m_digest[1] = m_digest[2] = m_digest[3] = 0;
    }
}

void phile_c::read_block()
{
    if ( m_no_more_blocks )
    {
        std::cout << "error " << m_name << " " << m_offset << " " 
                                    << m_size << std::endl;
    }
    assert( !m_no_more_blocks );
    if ( not m_ifs.is_open() )
    {
        try
        {
            m_ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            m_ifs.open(m_name, std::ios::binary | std::ios::ate);
        }
        catch(std::ifstream::failure e)
        {
            std::cout << "ifs open " << m_name << " failed " << e.what() 
                      << " open files " << s_open_files << "\n";
            m_no_more_blocks = true;
        }
    }
    if ( !m_ifs.is_open() )
    {
        return;
    }
    s_open_files++;
    try
    {
        m_ifs.seekg(m_offset);
    }
    catch(...)
    {
        std::cout << "exception in ifs.seekg\n";
    }
    size_t read_cnt = m_block_size;
    if ( m_size < (m_offset + m_block_size))
    {
        m_no_more_blocks = true;
        read_cnt = m_size - m_offset;
        // no need for padding m_block with zeroes
        // since initially m_block is already set to 0
        // and if we successfully going to the next compare tour
        // means m_blocks are equal ( not zero but equal )
    }
    m_offset += m_block_size;
    try
    {
        m_ifs.read(&(m_block[0]), read_cnt);
    }
    catch(std::bad_cast bc)
    {
        std::cout << "exception bad cast " << bc.what() << std::endl;
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "exception in ifs.read " << e.what() << std::endl;
    }

    m_ifs.close();
    s_open_files--;

    calc_digest();
}

boost::uint32_t optimal_crc32( void const *  buffer,
                                std::size_t   byte_count )
{
    static  boost::crc_32_type  computer;
    computer.reset();
    computer.process_bytes( buffer, byte_count );
    return computer.checksum();
}

void phile_c::calc_crc32()
{
    static  boost::crc_32_type  computer;
    computer.reset();
    computer.process_bytes( &(m_block[0]), m_block_size );
    m_digest[0] = computer.checksum();
}

void phile_c::calc_md5()
{
    boost::uuids::detail::md5 _md5;
    _md5.process_bytes(&(m_block[0]), m_block_size);
    _md5.get_digest(m_digest);
}

void phile_c::calc_digest()
{
    if (m_algo_md5)
    {
        calc_md5();
    }
    else
    {
        calc_crc32();
    }
}

phile_c::~phile_c()
{
    if ( m_ifs.is_open() )
    {
        m_ifs.close();
        s_open_files--;
    }
}

const std::string phile_c::get_name() const
{
    return m_name;
}

const unsigned int phile_c::get_size() const
{
    return m_size;
}

digest_key_c phile_c::get_block_hash()
{
    return digest_key_c(m_digest);
}

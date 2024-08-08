#define BOOST_TEST_MODULE bayan test

#include <fstream>
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "comparer.h"

namespace fs = boost::filesystem;

void fill_file(std::fstream &f)
{
    int i;
    for( i = 0; i < 100000; i++ )
    {
        f << "qqq aaa\n";
    }
    f.close();
}

/*
same files names started with "s"
unique files names started with "u"
    test_bayan
*/
void create_files()
{
     if ( fs::exists("./test_bayan") )
     {
        fs::remove_all("./test_bayan");
     }
     fs::create_directory("./test_bayan");
     std::fstream f("./test_bayan/s1", std::ios::out | std::ios::in | std::ios::trunc);
     fill_file(f);
     fs::create_directory("./test_bayan/sub");
     fs::copy_file("./test_bayan/s1", "./test_bayan/sub/s2");
}

void search_twins(sp_comparer_t sp_comparer)
{
    sp_comparer->open_directories();
    sp_comparer->find_twins();
}

bool compare_results(sp_comparer_t sp_comparer)
{
    fs::path abs1 = fs::canonical("./test_bayan/s1");
    fs::path abs2 = fs::canonical("./test_bayan/sub/s2");
    std::vector<std::string> similars;
    similars.push_back(abs1.native());
    similars.push_back(abs2.native());

    for ( auto s : similars ) std::cout << s << std::endl;
    std::cout << std::endl;
    for ( auto ss : sp_comparer->m_twins )
    {
        for ( auto s1 : ss )
        {
            std::cout << s1 << std::endl;
        }
    }
    return sp_comparer->has_twins(similars);
}

bool test_bayan()
{
    create_files();
    std::cout << "files created\n";
    sp_comparer_t sp_comparer{ new comparer_c() };
    string_vector_t sv {"./test_bayan"};
    sp_comparer->set_dirs(sv);
    sp_comparer->set_recursive(1);
    search_twins(sp_comparer);
    return compare_results(sp_comparer);
}

BOOST_AUTO_TEST_CASE(bayan_test)
{
    BOOST_CHECK( test_bayan() );
}

#include "comparer.h"

int main(int c, char *args[])
{
    sp_comparer_t sp_comparer{ new comparer_c() };
    if (!sp_comparer->get_options(c, args))
    {
        return -1;
    }
    sp_comparer->open_directories();
    sp_comparer->find_twins();
    sp_comparer->show_twins();
    return 0;
}


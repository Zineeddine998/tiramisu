#include <iostream>
#include <sstream>

namespace tiramisu
{

int tiramisu_indentation = 0;

void str_dump(const std::string &str)
{
    std::cout << str;
}

void str_dump(const std::string &str, const char *str2)
{
    std::cout << str << " " << str2;
}

void str_dump(const char *str, const char *str2)
{
    std::cout << str << " " << str2;
}

void print_indentation()
{
    for (int tiramisu_indent = 0; tiramisu_indent < tiramisu::tiramisu_indentation; tiramisu_indent++)
    {
        if (tiramisu_indent % 4 == 0)
        {
            str_dump("|");
        }
        else
        {
            str_dump(" ");
        }
    }
}

}

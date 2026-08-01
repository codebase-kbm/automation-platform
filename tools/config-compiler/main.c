#include <stdio.h>

#include "compiler.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(
            stderr,
            "Usage: %s <input.xml> <output.bin>\n",
            argv[0]
        );

        return 1;
    }

    return ap_config_compile(argv[1], argv[2]);
}
#include "smvsrecon.h"

int
main (int argc, char** argv)
{
    util::system::register_segfault_handler();
    util::system::print_build_timestamp("Shading-aware Multi-view Stereo");

    AppSettings conf = args_to_settings(argc, argv);
    std::cout << std::endl;

    perform(conf);
}
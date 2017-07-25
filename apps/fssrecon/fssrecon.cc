#include "fssrecon.h"

int
main (int argc, char** argv)
{
    util::system::register_segfault_handler();
    util::system::print_build_timestamp("Floating Scale Surface Reconstruction");

    /* Setup argument parser. */
    util::Arguments args;
    args.set_exit_on_error(true);
    args.set_nonopt_minnum(2);
    args.set_helptext_indent(25);
    args.set_usage(argv[0], "[ OPTS ] IN_PLY [ IN_PLY ... ] OUT_PLY");
    args.add_option('s', "scale-factor", true, "Multiply sample scale with factor [1.0]");
    args.add_option('r', "refine-octree", true, "Refines octree with N levels [0]");
    args.add_option('\0', "min-scale", true, "Minimum scale, smaller samples are clamped");
    args.add_option('\0', "max-scale", true, "Maximum scale, larger samples are ignored");
#if FSSR_USE_DERIVATIVES
    args.add_option('\0', "interpolation", true, "Interpolation: linear, scaling, lsderiv, [cubic]");
#endif // FSSR_USE_DERIVATIVES
    args.set_description("Samples the implicit function defined by the input "
        "samples and produces a surface mesh. The input samples must have "
        "normals and the \"values\" PLY attribute (the scale of the samples). "
        "Both confidence values and vertex colors are optional. The final "
        "surface should be cleaned (sliver triangles, isolated components, "
        "low-confidence vertices) afterwards.");
    args.parse(argc, argv);

    /* Init default settings. */
    AppOptions_fssr app_opts;
    fssr::SampleIO::Options pset_opts;

    /* Scan arguments. */
    while (util::ArgResult const* arg = args.next_result())
    {
        if (arg->opt == nullptr)
        {
            app_opts.in_files.push_back(arg->arg);
            continue;
        }

        if (arg->opt->lopt == "scale-factor")
            pset_opts.scale_factor = arg->get_arg<float>();
        else if (arg->opt->lopt == "refine-octree")
            app_opts.refine_octree = arg->get_arg<int>();
        else if (arg->opt->lopt == "min-scale")
            pset_opts.min_scale = arg->get_arg<float>();
        else if (arg->opt->lopt == "max-scale")
            pset_opts.max_scale = arg->get_arg<float>();
        else if (arg->opt->lopt == "interpolation")
        {
            if (arg->arg == "linear")
                app_opts.interp_type = fssr::INTERPOLATION_LINEAR;
            else if (arg->arg == "scaling")
                app_opts.interp_type = fssr::INTERPOLATION_SCALING;
            else if (arg->arg == "lsderiv")
                app_opts.interp_type = fssr::INTERPOLATION_LSDERIV;
            else if (arg->arg == "cubic")
                app_opts.interp_type = fssr::INTERPOLATION_CUBIC;
            else
            {
                args.generate_helptext(std::cerr);
                std::cerr << std::endl << "Error: Invalid interpolation: "
                    << arg->arg << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "Invalid option: " << arg->opt->sopt << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (app_opts.in_files.size() < 2)
    {
        args.generate_helptext(std::cerr);
        return EXIT_FAILURE;
    }
    app_opts.out_mesh = app_opts.in_files.back();
    app_opts.in_files.pop_back();

    if (app_opts.refine_octree < 0 || app_opts.refine_octree > 3)
    {
        std::cerr << "Unreasonable refine level of "
            << app_opts.refine_octree << ", exiting." << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        fssrecon(app_opts, pset_opts);
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "All done. Remember to clean the output mesh." << std::endl;

    return EXIT_SUCCESS;
}

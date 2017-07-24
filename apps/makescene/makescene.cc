#include "makescene.h"

int
main (int argc, char** argv)
{
    util::system::register_segfault_handler();
    util::system::print_build_timestamp("MVE Makescene");

    /* Setup argument parser. */
    util::Arguments args;
    args.set_usage(argv[0], "[ OPTIONS ] INPUT OUT_SCENE");
    args.set_exit_on_error(true);
    args.set_nonopt_maxnum(2);
    args.set_nonopt_minnum(2);
    args.set_helptext_indent(22);
    args.set_description("This utility creates MVE scenes by importing "
        "from an external SfM software. Supported are Noah's Bundler, "
        "Photosynther and VisualSfM's compact .nvm file.\n\n"

        "For VisualSfM, makescene expects the .nvm file as INPUT. "
        "With VisualSfM, it is not possible to keep invalid views.\n\n"

        "For Noah's Bundler, makescene expects the bundle directory as INPUT, "
        "a file \"list.txt\" in INPUT and the bundle file in the "
        "\"bundle\" directory.\n\n"

        "For Photosynther, makescene expects as INPUT the directory that "
        "contains the \"bundle\" and the \"undistorted\" directory. "
        "With Photosynther, it is not possible to keep invalid views "
        "or import original images.\n\n"

        "With the \"images-only\" option, all images in the INPUT directory "
        "are imported without camera information. If \"append-images\" is "
        "specified, images are added to an existing scene.");
    args.add_option('o', "original", false, "Import original images");
    args.add_option('b', "bundle-id", true, "Bundle ID (Photosynther and Bundler only) [0]");
    args.add_option('k', "keep-invalid", false, "Keeps images with invalid cameras");
    args.add_option('i', "images-only", false, "Imports images from INPUT_DIR only");
    args.add_option('a', "append-images", false, "Appends images to an existing scene");
    args.add_option('m', "max-pixels", true, "Limit image size by iterative half-sizing");
    args.parse(argc, argv);

    /* Setup defaults. */
    AppSettings conf;
    conf.input_path = util::fs::sanitize_path(args.get_nth_nonopt(0));
    conf.output_path = util::fs::sanitize_path(args.get_nth_nonopt(1));

    /* General settings. */
    for (util::ArgResult const* i = args.next_option();
        i != nullptr; i = args.next_option())
    {
        if (i->opt->lopt == "original")
            conf.import_orig = true;
        else if (i->opt->lopt == "bundle-id")
            conf.bundle_id = i->get_arg<int>();
        else if (i->opt->lopt == "keep-invalid")
            conf.skip_invalid = false;
        else if (i->opt->lopt == "images-only")
            conf.images_only = true;
        else if (i->opt->lopt == "append-images")
            conf.append_images = true;
        else if (i->opt->lopt == "max-pixels")
            conf.max_pixels = i->get_arg<int>();
        else
            throw std::invalid_argument("Unexpected option");
    }

    /* Check command line arguments. */
    if (conf.input_path.empty() || conf.output_path.empty())
    {
        args.generate_helptext(std::cerr);
        return EXIT_FAILURE;
    }
    conf.views_path = util::fs::join_path(conf.output_path, VIEWS_DIR);
    conf.bundle_path = util::fs::join_path(conf.input_path, BUNDLE_PATH);

    if (conf.append_images && !conf.images_only)
    {
        std::cerr << "Error: Cannot --append-images without --images-only."
            << std::endl;
        return EXIT_FAILURE;
    }

    /* Check if output dir exists. */
    bool output_path_exists = util::fs::dir_exists(conf.output_path.c_str());
    if (output_path_exists && !conf.append_images)
    {
        std::cerr << std::endl;
        std::cerr << "** Warning: Output dir already exists." << std::endl;
        std::cerr << "** This may leave old views in your scene." << std::endl;
        wait_for_user_confirmation();
    }
    else if (!output_path_exists && conf.append_images)
    {
        std::cerr << "Error: Output dir does not exist. Cannot append images."
            << std::endl;
        return EXIT_FAILURE;
    }

    if (conf.images_only)
        import_images(conf);
    else
        import_bundle(conf);

    return EXIT_SUCCESS;
}

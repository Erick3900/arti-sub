#include "options_parser.hpp"

namespace arti {

    options_parser::options_parser()
        : m_Options{ "Artichoke Substitution Tool:\n\nUsage:" } {
        auto optionsDef = m_Options.add_options();

        optionsDef("version,v", "Prints the program version information");
        optionsDef("include,I", opt::value<std::string>(), "Include path");
        optionsDef("file,f", opt::value<std::string>(), "File path");
        optionsDef("output,o", opt::value<std::string>(), "Output file");
        optionsDef("help,h", "Prints this help message");
    }

    options_parser::expected_t options_parser::parse(int argc, char **argv) {
        opt::variables_map vars;

        try {
            opt::store(opt::parse_command_line(argc, argv, m_Options), vars);

            if (vars.empty()) {
                return expected_t::unexpected_type{ { errors::Empty, "No params provided" } };
            }

            if (vars.contains("help")) {
                return expected_t::unexpected_type{ { errors::Help, help() } };
            }

            opt::notify(vars);
        }
        catch(std::exception &err) {
            return expected_t::unexpected_type{ { errors::Invalid, err.what() } };
        }

        return std::move(vars);
    }

    std::string options_parser::help() const {
        return (std::stringstream{} << m_Options).str();
    }

}

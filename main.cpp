#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include <internal/config.hpp>

#include <ctre.hpp>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include "options_parser.hpp"

namespace fs = std::filesystem;

void printVersion();

std::set<std::string> includeLines;

tl::expected<std::string, std::string> substitute(fs::path file, fs::path includePath) {
    auto printIfNotEmpty = [] (std::ostream &os, std::string_view line) {
        if (! line.empty()) {
            os << line << '\n';
        }
    };

    std::size_t lineCount = 0;

    std::stringstream output;
    std::string line;

    std::ifstream fileIS{ file };

    if (! fileIS.is_open()) {
        return tl::unexpected<std::string>{ fmt::format("Coudln't open the file {}.", file.string()) };
    }

    while (std::getline(fileIS, line)) {
        if (ctre::match<"#pragma (.+)">(line)) {
            continue;
        }

        auto match = ctre::match<"#include[ ]?\\\"(?<file>.+)\\\"(?<comment>[ ]?(//|/\\*).+)?">(line);

        if (match) {
            auto includeFile = match.get<"file">().str();
            auto includeFilePath = includePath / includeFile;
            auto comment = match.get<"comment">().str();

            if (! fs::exists(includeFilePath)) {
                return tl::unexpected<std::string>{ 
                    fmt::format("File '{}' included in line {} of file '{}' not found.",
                        includeFile,
                        lineCount,
                        file.string()
                    )
                };
            }

            if (includeLines.contains(includeFile)) {
                std::cerr << fmt::format("Ignoring the include of file '{}' in line {} of file '{}'",
                    includeFile, 
                    lineCount,
                    file.string()
                ) << std::endl;

                printIfNotEmpty(output, comment);
                continue;
            }

            auto recursiveSubs = substitute(includeFilePath, includePath);

            if (! recursiveSubs) {
                return std::move(recursiveSubs).error();
            }

            includeLines.insert(includeFile);

            output << fmt::format("// File: '{}'\n", includeFile);
            output << recursiveSubs.value() << '\n';
            output << fmt::format("// EOF: {}\n", includeFile);
            printIfNotEmpty(output, comment);
        }
        else {
            auto match = ctre::match<"#include[ ]?[<](?<include>.+)[>](?<comment>[ ]?(//|/\\*).+)?">(line);
            if (match) {
                auto includeLine = match.get<"include">().str();

                if (includeLines.contains(includeLine)) {
                    std::cerr << fmt::format("Ignoring include of <{}>", includeLine) << std::endl;
                    printIfNotEmpty(output, match.get<"comment">().str());
                }
                else {
                    includeLines.insert(includeLine);
                    output << line << '\n';
                }
            }
            else {
                output << line << '\n';
            }
        }
        
        ++lineCount;
    }

    return output.str();
}

int main(int argc, char* argv[]) {
    auto parserEx = [&] {
        arti::options_parser options;

        return options.parse(argc, argv);
    }();

    if (! parserEx) {
        auto &&[errorCode, errorInfo] = std::move(parserEx).error();
    
        switch (errorCode) {
            case decltype(errorCode)::Empty:
                fmt::print("{}, see usage with '--help' or '-h'\n", errorInfo);
                break;
            case decltype(errorCode)::Help:
                fmt::print("{}\n", errorInfo);
                break;
            case decltype(errorCode)::Invalid:
                fmt::print("Invalid params provided: '{}'\n", errorInfo);
                break;
        }

        return 1;
    }

    auto options = std::move(parserEx).value();

    if (options.contains("version")) {
        printVersion();
        return 1;
    }

    if (! options.contains("file") || ! options.contains("include")) {
        fmt::print("Both parameters 'include' and 'file' are required\n");
        return 1;
    }

    namespace fs = std::filesystem;

    auto pFile = options.at("file").as<std::string>();
    fs::path pInclude = options.at("include").as<std::string>();
   
    auto subsExp = substitute(pFile, pInclude);

    if (! subsExp) {
        fmt::print("Error, {}\n", subsExp.error());

        return 1;
    }

    auto runOutput = [&] (std::ostream &os) {
        os << subsExp.value() << std::endl;
    };

    if (options.contains("output")) {
        std::ofstream outputFile{ options.at("output").as<std::string>() };

        if (outputFile.is_open()) {
            runOutput(outputFile);
        }
        else {
            std::cerr << fmt::format("Coudln't open output file '{}', printing output to stdout", options.at("output").as<std::string>()) << std::endl;
            runOutput(std::cout);
        }
    }
    else {
        runOutput(std::cout);
    }
}

void printVersion() {
    using namespace arti;

    fmt::print(
        "{} version {}\n\n"
        "       Author:  {}\n"
        "Author Github: {}\n",
        config::project_name,
        config::project_version,
        config::project_author,
        config::project_author_github
    );
}


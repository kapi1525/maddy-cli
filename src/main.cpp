#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>
#include <variant>
#include <filesystem>

#include "maddy/parser.h"


// Usage maddy-cli [FILE] [OPTIONS...]\n
//
// Options:
// --noemphasizedtext             Disables emphasized text support '_emphasized text_'.
// --nowraphtmlinparagraph        If line starts with '<', line wont be wrapped inside '<p>'.
// --output=<file>                Write html to file instead of stdout.



void print_usage() {
    const char* usage = "Usage maddy-cli [FILE] [OPTIONS...]\n"
                        "\n"
                        "Options:\n"
                        "--noemphasizedtext             Disables emphasized text support '_emphasized text_'.\n"
                        "--nowraphtmlinparagraph        If line starts with '<', line wont be wrapped inside '<p>'.\n"
                        "--output=<file>                Write html to file instead of stdout.\n\n";
    std::printf("%s", usage);
}



class options {
public:
    options() = default;
    ~options() = default;

    // Register option
    void register_option(std::string option, std::function<void()> func) {
        registered_options.push_back({option, func});
    }

    // Register option with string argument
    void register_option(std::string option, std::function<void(const std::string&)> func) {
        registered_options.push_back({option, func});
    }

    // Register handler for things that are not options
    void register_other(std::function<void(const std::string&)> func) {
        other = func;
    }


    // Parse for options and call functions if found
    // false if unknown option
    bool parse(int argc, const char *argv[]) {
        std::vector<std::string> args;

        for (int i = 1; i < argc; i++) {
            args.push_back(argv[i]);
        }


        for (size_t i = 0; i < args.size(); i++) {
            if(args[i].compare(0, 2, "--") == 0) {
                bool found = false;

                for (auto& opt : registered_options) {
                    if(args[i].compare(2, std::get<0>(opt).size(), std::get<0>(opt)) == 0) {
                        found = true;

                        if(std::get<1>(opt).index() == 0) {
                            std::get<0>(std::get<1>(opt))();
                        }
                        else {
                            if(2 + std::get<0>(opt).size() +2 > args[i].size() || args[i][2 + std::get<0>(opt).size() +1] == '=') {
                                return false;
                            }
                            std::get<1>(std::get<1>(opt))(args[i].substr(2 + std::get<0>(opt).size() +1));
                        }

                        break;
                    }
                }

                if(!found) {
                    return false;
                }

            }
            else {
                if(other) {
                    other(args[i]);
                } else {
                    return false;
                }
            }
        }


        return true;
    }


private:
    std::vector<std::tuple<std::string, std::variant<std::function<void()>, std::function<void(const std::string&)>>>> registered_options;
    std::function<void(const std::string&)> other;

};




int main(int argc, const char *argv[]) {
    options options_parser;

    std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
    std::filesystem::path filepath;
    std::filesystem::path outputfilepath;


    // Register commandline options
    options_parser.register_option("noemphasizedtext", [&]() {
        config->isEmphasizedParserEnabled = false;
    });

    options_parser.register_option("nowraphtmlinparagraph", [&]() {
        config->isHTMLWrappedInParagraph = false;
    });


    options_parser.register_option("output", [&](const std::string& str) {
        outputfilepath = std::filesystem::absolute(str);
    });

    options_parser.register_other([&](const std::string& str) {
        filepath = std::filesystem::absolute(str);
    });



    // Parse commandline
    if(!options_parser.parse(argc, argv) || filepath.empty()) {
        print_usage();
        return 0;
    }


    if(!std::filesystem::exists(filepath)) {
        std::printf("File doesnt exist: '%s'.\n", filepath.c_str());
        return 0;
    }


    // Openfile
    std::ifstream filestream(filepath, std::ios::binary);

    if(!filestream.good()) {
        std::printf("Error reading file: '%s'.\n", filepath.c_str());
        return 0;
    }


    // Convert markdown to html
    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
    std::string html = parser->Parse(filestream);

    filestream.close();


    if(outputfilepath.empty()) {
        std::printf("%s\n", html.c_str());
    } else {
        std::ofstream outputfilestream(outputfilepath, std::ios::binary);

        if(!outputfilestream.good()) {
            std::printf("Error writing file: '%s'.\n", outputfilepath.c_str());
            return 0;
        }

        outputfilestream << html;
    }
    return 0;
}

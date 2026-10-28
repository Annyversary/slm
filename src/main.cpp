#include "NGramModel.h"
#include "TextGenerator.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Helper: parse a strictly positive integer from a C-string.
// Throws std::invalid_argument with a descriptive message on failure.
// ---------------------------------------------------------------------------
static int parsePositiveInt(const char* s, const std::string& name)
{
    if (s == nullptr || s[0] == '\0') {
        throw std::invalid_argument(name + " must be a positive integer");
    }

    std::size_t pos = 0;
    int value = 0;
    try {
        value = std::stoi(std::string(s), &pos);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            name + " must be a positive integer, got: " + std::string(s));
    }

    // Ensure the entire string was consumed (reject e.g. "2.5" or "2abc").
    if (pos != std::string(s).size()) {
        throw std::invalid_argument(
            name + " must be a positive integer, got: " + std::string(s));
    }
    if (value <= 0) {
        throw std::invalid_argument(
            name + " must be a positive integer, got: " + std::string(s));
    }
    return value;
}

// ---------------------------------------------------------------------------
// Helper: parse a non-negative double from a C-string.
// Throws std::invalid_argument with a descriptive message on failure.
// ---------------------------------------------------------------------------
static double parseNonNegativeDouble(const char* s)
{
    if (s == nullptr || s[0] == '\0') {
        throw std::invalid_argument(
            "option -a requires a non-negative number");
    }

    std::size_t pos = 0;
    double value = 0.0;
    try {
        value = std::stod(std::string(s), &pos);
    } catch (const std::exception&) {
        throw std::invalid_argument(
            std::string("option -a requires a non-negative number, got: ") + s);
    }

    if (pos != std::string(s).size()) {
        throw std::invalid_argument(
            std::string("option -a requires a non-negative number, got: ") + s);
    }
    if (value < 0.0) {
        throw std::invalid_argument(
            std::string("pseudocount must be non-negative, got: ") + s);
    }
    return value;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try {
        // ----------------------------------------------------------------
        // Argument parsing
        // ----------------------------------------------------------------
        int    argIdx = 1;
        double alpha  = 0.0;

        // Optional -a <value> before the three positional arguments.
        if (argIdx < argc) {
            std::string first(argv[argIdx]);
            if (first == "-a") {
                ++argIdx;
                if (argIdx >= argc) {
                    throw std::invalid_argument(
                        "option -a requires a non-negative number");
                }
                alpha = parseNonNegativeDouble(argv[argIdx]);
                ++argIdx;
            } else if (!first.empty() && first[0] == '-') {
                throw std::invalid_argument(
                    "unknown option: " + first +
                    "\nusage: slm [-a alpha] k file n");
            }
        }

        // Exactly three positional arguments must remain.
        if (argc - argIdx != 3) {
            throw std::invalid_argument(
                "wrong number of arguments\n"
                "usage: slm [-a alpha] k file n");
        }

        const int         k          = parsePositiveInt(argv[argIdx],     "k");
        const std::string filename   = argv[argIdx + 1];
        const int         outputLen  = parsePositiveInt(argv[argIdx + 2], "output length");

        if (outputLen < k) {
            throw std::invalid_argument(
                "output length (" + std::to_string(outputLen) +
                ") must be at least k (" + std::to_string(k) + ")");
        }

        // ----------------------------------------------------------------
        // Read training file
        // ----------------------------------------------------------------
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error(
                "cannot open file '" + filename + "'");
        }
        const std::string text(
            (std::istreambuf_iterator<char>(file)),
             std::istreambuf_iterator<char>());
        file.close();

        // ----------------------------------------------------------------
        // Build model and generate text
        // ----------------------------------------------------------------
        NGramModel    model(k, text, alpha);
        TextGenerator generator(model);

        std::cout << generator.generate(outputLen) << "\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "slm: " << e.what() << "\n";
        return 1;
    }
}

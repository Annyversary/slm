#include "TextGenerator.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

TextGenerator::TextGenerator(const NGramModel& model, unsigned int seed)
    : model_(model), rng_(seed)
{}

// ---------------------------------------------------------------------------
// generate
// ---------------------------------------------------------------------------

std::string TextGenerator::generate(int numChars) const
{
    const int k = model_.getK();

    // Step 1 & 2: choose and output the starting k-gram.
    std::string current = model_.sampleStartNgram(rng_);
    std::string result  = current;

    // Steps 3–5: extend the output one character at a time.
    for (int generated = k; generated < numChars; ++generated) {
        char next = model_.sampleNextChar(current, rng_);
        if (next == '\0') {
            break;  // stuck — give up
        }
        result  += next;
        // Slide the window: drop first character, append the new one.
        current  = current.substr(1) + next;
    }

    return result;
}

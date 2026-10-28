#pragma once

#include "NGramModel.h"
#include <string>
#include <random>

/**
 * TextGenerator
 *
 * Generates text using a trained NGramModel.
 *
 * Algorithm (per spec):
 *   1. Sample a starting k-gram proportionally to its frequency.
 *   2. Output it.
 *   3. Sample the next character c from the conditional distribution
 *      P(c | current_ngram).  If alpha==0 and no transitions exist, stop.
 *   4. Output c; advance the sliding window (drop first char, append c).
 *   5. Repeat until the requested number of characters have been output
 *      or generation gets stuck.
 *
 * The output length includes the starting k-gram.
 */
class TextGenerator {
public:
    /**
     * Construct a generator for the given model.
     *
     * @param model  A fully trained NGramModel.
     * @param seed   Seed for the PRNG (default: a non-deterministic seed).
     */
    explicit TextGenerator(const NGramModel& model,
                           unsigned int seed = std::random_device{}());

    /**
     * Generate up to numChars characters.
     * The result may be shorter than numChars if the generator gets stuck.
     *
     * @param numChars  Total number of characters to output (>= k).
     */
    std::string generate(int numChars) const;

private:
    const NGramModel&    model_;
    mutable std::mt19937 rng_;   // mutable so generate() can be const-callable
};

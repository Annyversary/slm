#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <random>

/**
 * NGramModel
 *
 * Trains a k-gram language model on a given input text.
 * Stores:
 *   - k-gram occurrence counts (for sampling a starting k-gram)
 *   - character transition counts per k-gram
 *   - the alphabet (set of distinct characters in the training text)
 *
 * Supports an optional Laplace-style pseudocount alpha >= 0.
 * With alpha == 0, k-grams that only appear at the very end of the text
 * (and thus have no transitions) cause generation to stop.
 * With alpha > 0, every k-gram has a positive probability for every
 * alphabet character (uniform for unseen k-grams).
 */
class NGramModel {
public:
    /**
     * Construct and train the model.
     *
     * @param k      Length of each n-gram (must be >= 1).
     * @param text   The training text (plain ASCII assumed).
     * @param alpha  Pseudocount (>= 0, default 0).
     *
     * @throws std::invalid_argument if k < 1.
     * @throws std::runtime_error    if the text is too short to contain
     *                               any k-gram followed by a character.
     */
    NGramModel(int k, const std::string& text, double alpha = 0.0);

    /** Return the k-gram length. */
    int getK() const { return k_; }

    /**
     * Sample a starting k-gram proportionally to its frequency in the
     * training text.
     */
    std::string sampleStartNgram(std::mt19937& rng) const;

    /**
     * Sample the next character given a current k-gram.
     *
     * Returns '\0' (stop signal) when alpha == 0 and the k-gram has no
     * recorded transitions (i.e., it only appeared at the very end of
     * the training text, or it was never seen at all).
     */
    char sampleNextChar(const std::string& ngram, std::mt19937& rng) const;

private:
    int    k_;
    double alpha_;

    // Ordered alphabet (for stable indexing in discrete_distribution)
    std::vector<char> alphabet_;

    // ngram -> total number of transitions from it (M_k in the spec)
    std::map<std::string, int> transition_total_;

    // ngram -> char -> count  (f_{k,c} in the spec)
    std::map<std::string, std::map<char, int>> transitions_;

    // For sampling the starting k-gram
    std::vector<std::string> ngram_list_;
    std::vector<double>      ngram_weights_;
};

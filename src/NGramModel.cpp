#include "NGramModel.h"

#include <stdexcept>
#include <set>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

NGramModel::NGramModel(int k, const std::string& text, double alpha)
    : k_(k), alpha_(alpha)
{
    if (k < 1) {
        throw std::invalid_argument("k must be a positive integer");
    }

    // The text must contain at least one k-gram that is followed by a
    // character, i.e. the text length must be at least k+1.
    if (static_cast<int>(text.size()) < k + 1) {
        throw std::runtime_error(
            "training text is too short to contain any " +
            std::to_string(k) + "-gram followed by a character");
    }

    // -----------------------------------------------------------------
    // Build alphabet
    // -----------------------------------------------------------------
    std::set<char> alpha_set(text.begin(), text.end());
    alphabet_ = std::vector<char>(alpha_set.begin(), alpha_set.end());

    // -----------------------------------------------------------------
    // Count k-gram occurrences and character transitions
    // -----------------------------------------------------------------
    // ngram_counts holds ALL occurrences (including those at the very end).
    std::map<std::string, int> ngram_counts;

    const std::size_t n = text.size();
    for (std::size_t i = 0; i + static_cast<std::size_t>(k) <= n; ++i) {
        std::string ngram = text.substr(i, k);
        ngram_counts[ngram]++;

        // Only count transition if this k-gram is NOT at the very end
        if (i + static_cast<std::size_t>(k) < n) {
            char next = text[i + k];
            transitions_[ngram][next]++;
            transition_total_[ngram]++;
        }
    }

    // Verify that there is at least one k-gram with a transition.
    // (Redundant given the length check above, but kept for clarity.)
    if (transitions_.empty()) {
        throw std::runtime_error(
            "training text is too short to contain any " +
            std::to_string(k) + "-gram followed by a character");
    }

    // -----------------------------------------------------------------
    // Build start-ngram sampling distribution (weighted by frequency)
    // -----------------------------------------------------------------
    ngram_list_.reserve(ngram_counts.size());
    ngram_weights_.reserve(ngram_counts.size());

    for (auto& [ngram, count] : ngram_counts) {
        ngram_list_.push_back(ngram);
        ngram_weights_.push_back(static_cast<double>(count));
    }
}

// ---------------------------------------------------------------------------
// sampleStartNgram
// ---------------------------------------------------------------------------

std::string NGramModel::sampleStartNgram(std::mt19937& rng) const
{
    std::discrete_distribution<std::size_t> dist(
        ngram_weights_.begin(), ngram_weights_.end());
    return ngram_list_[dist(rng)];
}

// ---------------------------------------------------------------------------
// sampleNextChar
// ---------------------------------------------------------------------------

char NGramModel::sampleNextChar(const std::string& ngram, std::mt19937& rng) const
{
    auto it = transitions_.find(ngram);

    if (alpha_ == 0.0) {
        // No pseudocount: stop if there are no transitions.
        if (it == transitions_.end() || it->second.empty()) {
            return '\0';
        }

        // Build weight vector only over characters that actually appear.
        std::vector<char>   chars;
        std::vector<double> weights;
        for (auto& [c, cnt] : it->second) {
            chars.push_back(c);
            weights.push_back(static_cast<double>(cnt));
        }

        std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
        return chars[dist(rng)];

    } else {
        // With pseudocount: every alphabet character gets positive weight.
        // P(c | ngram) = (f_{ngram,c} + alpha) / (M_ngram + alpha * |alphabet|)
        // The denominator is constant across all characters, so it cancels out
        // when constructing std::discrete_distribution — only the numerators
        // (f_{ngram,c} + alpha) are needed as weights.
        std::vector<double> weights;
        weights.reserve(alphabet_.size());

        for (char c : alphabet_) {
            double f = 0.0;
            if (it != transitions_.end()) {
                auto jt = it->second.find(c);
                if (jt != it->second.end()) {
                    f = static_cast<double>(jt->second);
                }
            }
            weights.push_back(f + alpha_);
        }

        std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
        return alphabet_[dist(rng)];
    }
}

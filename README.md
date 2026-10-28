# slm — Small Language Model

A command-line tool that trains a k-gram character-level language model on a
plain-text file and generates new text from it.  It is deliberately simple (an
"Artificial Idiot") but the statistical foundation is solid: frequencies,
conditional probability distributions with optional Laplace-style
pseudocounts, and a sliding-window text generator.

---

## Building

Requirements: a C++17-capable compiler (e.g. `g++ 9` or later) and `make`.

```sh
# From the project root:
make
```

This compiles all sources under `src/` into `obj/*.o` and links them into the
`slm` executable in the project root.

```sh
make clean    # removes obj/ and the slm executable
```

---

## Usage

```
slm [-a alpha] k file n
```

| Parameter | Description |
|-----------|-------------|
| `k`       | N-gram length (positive integer) |
| `file`    | Path to the training text file |
| `n`       | Number of characters to output (must be ≥ k) |
| `-a alpha`| Optional pseudocount α ≥ 0 (default: 0) |

### Examples

Generate 5 characters using bigrams, no pseudocount:

```sh
./slm 2 moby_dick.txt 5
```

Example output: `it we`

Generate 40 characters using trigrams with pseudocount 0.01:

```sh
./slm -a 0.01 3 moby_dick.txt 40
```

---

## How it works

### Training

All overlapping k-grams are extracted from the input file.  For each k-gram
the tool records:

- its total frequency in the text (used to sample the starting k-gram), and
- for every character that follows it, the transition count.

### Conditional probability

Given k-gram *w* and alphabet Σ, the probability of the next character *c* is:

```
P(c | w) = (f(w,c) + α) / (M(w) + α · |Σ|)
```

where `f(w,c)` is the number of times *c* followed *w*, and `M(w)` is the
total number of transitions from *w*.

With `α = 0` (the default) this is the maximum-likelihood estimate.  With
`α > 0` (Laplace smoothing) every character receives a positive probability,
including characters that never followed *w* and even k-grams that never
appeared in the training text.

### Generation

1. Sample a starting k-gram proportionally to its frequency.
2. Repeatedly sample the next character from the conditional distribution,
   slide the window by one, and continue.
3. Stop when the requested length is reached, or when the generator gets
   stuck (only possible with `α = 0`).

---

## Project layout

```
slm/
├── Makefile          — build system
├── README.md         — this file / lab report
├── .gitignore
└── src/
    ├── NGramModel.h      — model class declaration
    ├── NGramModel.cpp    — model training & sampling
    ├── TextGenerator.h   — generator class declaration
    ├── TextGenerator.cpp — text generation loop
    └── main.cpp          — argument parsing & entry point
```

---

## Error handling

All user errors are caught by `main` and printed to `stderr`; the program
exits with status 1.  Detected errors include:

- Wrong number of arguments.
- `k` or output length is not a positive integer, or output length < k.
- Unknown option or `-a` not followed by a non-negative number.
- Training file cannot be opened.
- Training text too short for even one k-gram with a following character.

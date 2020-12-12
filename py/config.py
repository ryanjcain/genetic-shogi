import sys
sys.path.append('../cpp')


# --------------------------- Genetic Algorithm Framework ---------------------------

# Size of data sampled (whatever --n was passed to sample.py)
N_TRAIN = 5000
N_TEST = 5000

# Set the constants for our genetic algorithm
POP_SIZE = 100
CXPB = 0.75
MUTPB = 0.005
N_GEN = 200

# Bit widths to be used for ga
BIT_WIDTH_SMALL = 7
BIT_WIDTH_WIDE = 12  # Paper used 16 for pieces

LOG_FILE = "results.txt"
ORGANISM_SAVE_FILE = "organisms.txt"

VERBOSE = False  # Print out each organism's weights throughout evolution

EVAL_MODE = "train"

# Save all of the parameters into a list for easy import/exporting
params = {
    "eval_lang": "C++",
    "n_train": N_TRAIN,
    "n_test": N_TEST,
    "pop_size": POP_SIZE,
    "cxpb": CXPB,
    "mutpb": MUTPB,
    "n_gen": N_GEN,
    "bit_width_small": BIT_WIDTH_SMALL,
    "bit_width_wide": BIT_WIDTH_WIDE,
    "log_file": LOG_FILE,
    "verbose": VERBOSE,
    "eval_mode": EVAL_MODE,
    "organism_file": ORGANISM_SAVE_FILE,
}

import sys
sys.path.append('./cpp')


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
BIT_WIDTH_SMALL = 6
BIT_WIDTH_WIDE = 12  # Paper used 16 for pieces

LOG_FILE = "results.txt"
VERBOSE = True  # Print out each organism's weights throughout evolution

NUM_FEATURES = 24
NUM_PIECE_TYPES = 8

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
    "num_piece_types": NUM_PIECE_TYPES,
    "num_other_features": NUM_FEATURES,
    "log_file": LOG_FILE,
    "verbose": VERBOSE,
    "chromosome_len": (BIT_WIDTH_SMALL * NUM_FEATURES) +
                      (NUM_PIECE_TYPES * BIT_WIDTH_WIDE)
}

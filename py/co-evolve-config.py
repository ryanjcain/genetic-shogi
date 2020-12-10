# Number of games to simulate
N_SIMULATE = 4
POP_SIZE = 10
CXPB = 0.75
MUTPB = 0.005
N_GEN = 50


MAX_SEARCH_DEPTH = 100
MAX_TURNS = 200

LOG_FILE = ""

VERBOSE = False

# Save all of the parameters into a list for easy import/exporting
params = {
    "eval_lang": "C++",
    "pop_size": POP_SIZE,
    "cxpb": CXPB,
    "mutpb": MUTPB,
    "n_gen": N_GEN,
    "n_simulate": N_SIMULATE,
    "log_file": LOG_FILE,
    "verbose": VERBOSE,
    "organism_file": ORGANISM_FILE,
    "max_search_depth": MAX_SEARCH_DEPTH,
    "max_turns": MAX_TURNS,
}

# ---------------------------  Python or C++ Evaluation  ----------------------------
from features import ShogiFeatures
from sample import GameSampler
import progressbar
import itertools
import ctypes
import pickle
import shogi
EVAL_LANG = "C++"
# EVAL_LANG = "Python"

PY_MULTIPROCESSING = False  # Gives organism level parallelization
# Further game level thread concurrency (Open MP)
# default when using C++ evaluation (uses Open MP)

# --------------------------- Genetic Algorithm Framework ---------------------------
# Size of data sampled (whatever --n was passed to sample.py)
N = 10000

# Set the constants for our genetic algorithm
POP_SIZE = 100  # 100 (size used in GA chess paper)
CXPB = 0.75
MUTPB = 0.005
N_GEN = 200  # 200 (gens used in GA chess paper)

LOG_FILE = "results.txt"

############################# LIBRARIES, CACHING, LOGGING ############################
#############################        DO NOT CHANGE        ############################

# My libraries

# --------------------------- Helper functions and logger  ---------------------------

# Helpoer function to load in saved python objects for caching


def load_obj(name):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)


# Function to facilitate logging


def log(message, console=False, end="\n"):
    if not console:
        with open(LOG_FILE, "a") as f:
            f.write(message)
            f.write(end)
    else:
        print(message)


# --------------------------- Shogi Features & Heuristic  ---------------------------

legal_moves_cache, TRAIN, TEST = [], [], []
SHOGI_FEATURES, NUM_FEATURES, NUM_TRAIN = None, None, None

if EVAL_LANG == "Python":
    legal_moves_cache = load_obj("legal_moves_cache")
    TRAIN = load_obj("train_data")
    TEST = load_obj("test_data")

    # Initialize/extract all the feature functions
    SHOGI_FEATURES = ShogiFeatures().all()
    NUM_FEATURES = len(SHOGI_FEATURES)

elif EVAL_LANG == "C++":
    # Bring in the Cpp library
    CPP_SHOGI = ctypes.CDLL("./cpp/shogilib.so")
    NUM_FEATURES = 24
    NUM_PIECE_TYPES = 8

else:
    print("Unsupported evaluation language: {}".format(cfg.EVAL_LANG))
    print("Change to either 'C++' or 'Python' in config.py.")
    exit(1)

# Number of bits used to represent a single weight
BIT_WIDTH_SMALL = 6
BIT_WIDTH_WIDE = 12  # Paper used 16 for pieces

CHROMOSOME_LEN = (BIT_WIDTH_SMALL * NUM_FEATURES) + \
    (NUM_PIECE_TYPES * BIT_WIDTH_WIDE)

NUM_TRAIN = int(N / 2)

# --------------------------- Logging and Progress Bar ------------------------------

# Initialize a progress bar to be referenced from fitness func (helps with ETA)
widgets = [
    ' [',
    progressbar.Timer(), '] ',
    progressbar.Bar(), ' (',
    progressbar.AdaptiveETA(), ') ',
    progressbar.Percentage()
]
PROGRESS_BAR = progressbar.ProgressBar(maxval=N_GEN, widgets=widgets)



# ---------------------------  Python or C++ Evaluation  ----------------------------
EVAL_LANG = "C++"
# EVAL_LANG = "Python"

PY_MULTIPROCESSING = True       # Gives organism level parallelization
                                # Further game level thread concurrency (Open MP)
                                # default when using C++ evaluation (uses Open MP)

# --------------------------- Genetic Algorithm Framework --------------------------- 
# Size of data sampled (whatever --n was passed to sample.py)
N = 1000

# Set the constants for our genetic algorithm
POP_SIZE = 10               # 100 (size used in GA chess paper)
CXPB = 0.75
MUTPB = 0.005
N_GEN = 3                   # 200 (gens used in GA chess paper)


############################# LIBRARIES, CACHING, LOGGING ############################
#############################        DO NOT CHANGE        ############################

import shogi
import pickle
import ctypes
import itertools
import progressbar

# My libraries
from sample import GameSampler
from features import ShogiFeatures

# --------------------------- Helper functions and logger  ---------------------------

# Helpoer function to load in saved python objects for caching
def load_obj(name):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)

# Function to facilitate logging
def log(message, console=False, end="\n"):
    if not console:
        with open("log.txt", "a") as f:
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
    
    SHOGI_FEATURES = ShogiFeatures().all()           # Initialize/extract all the feature functions
    NUM_FEATURES = len(SHOGI_FEATURES)

elif EVAL_LANG == "C++":
    # Bring in the Cpp library
    CPP_SHOGI = ctypes.CDLL("./cpp/shogilib.so")
    NUM_FEATURES = 20

else:
    print("Unsupported evaluation language: {}".format(cfg.EVAL_LANG))
    print("Change to either 'C++' or 'Python' in config.py.")
    exit(1)

# Number of bits used to represent a single weight
BIT_WIDTH = 6                          

CHROMOSOME_LEN = BIT_WIDTH * NUM_FEATURES
NUM_TRAIN = int(N / 2)

# --------------------------- Logging and Progress Bar ------------------------------ 

# Initialize a progress bar to be referenced from fitness func (helps with ETA)
widgets=[' [', progressbar.Timer(), '] ', progressbar.Bar(),
        ' (', progressbar.AdaptiveETA(), ') ', progressbar.Percentage()]
PROGRESS_BAR = progressbar.ProgressBar(maxval=N_GEN, widgets=widgets)




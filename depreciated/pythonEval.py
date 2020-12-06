'''
ATTENTION!
This module is depreciated! Do not use!! Python evaluation was used with the features
described in features.py before migrating to cpp
'''

from features import ShogiFeatures

legal_moves_cache, TRAIN, TEST = [], [], []
SHOGI_FEATURES, NUM_FEATURES, NUM_TRAIN = None, None, None

legal_moves_cache = load_obj("legal_moves_cache")
TRAIN = load_obj("train_data")
TEST = load_obj("test_data")

# Initialize/extract all the feature functions
SHOGI_FEATURES = ShogiFeatures().all()
NUM_FEATURES = len(SHOGI_FEATURES)

# --------------------------- Helper Functions and other things.  ------------------------------


def load_obj(name):
    with open('obj/' + name + '.pkl', 'rb') as f:
        return pickle.load(f)


# --------------------------- Python implementation of evaluation  -----------------------------


# Heuristic function must have global scope for multiprocessing
def H(action, weights):
    '''
    This is the python version of the heuristic evaluation.
    '''
    move, resulting_pos = action[0], action[1]
    value = sum([
        cfg.SHOGI_FEATURES[i](resulting_pos) * weights[i]
        for i in range(cfg.NUM_FEATURES)
    ])
    return move, value


def pythonEval(weights):

    # Iterate through sample of grandmaster games to see peformance of heuristic
    correct = 0
    game_no = 0
    eval_tm = 0
    positions = 0

    for game in cfg.TRAIN:
        eval_start = time.time()
        # Initialize the board to the reference game
        pos, grandmaster_move = game[0], game[1]
        best_move, best_val = "", 0

        # Run 1-ply search by using a cache of precalculated legal moves
        for action in cfg.legal_moves_cache[pos]:
            # Evalueate the heuristic to see the value of the action
            move, value = H(action, weights)

            # Keep track of the best seen so far
            if value > best_val:
                best_move = move
                best_val = value

        # Update if the Heuristic guessed correctly
        if best_move == grandmaster_move:
            correct += 1

        game_no += 1
        positions += len(cfg.legal_moves_cache[pos])
        eval_tm += time.time() - eval_start

    fitness = correct * correct
    return fitness,

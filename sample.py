import argparse
import json
import pickle
import random
import sys
import re

import progressbar

import shogi

# Coppied from easyShogiCpp for reference in hex conversion
FOOT = 0
SILVER = 1
CASSIA = 2
CHARIOT = 3
FLYING = 4
ANGLE = 5
KING = 6
GOLD = 7

# Renaming for better allignment with python shogi
BISHOP = ANGLE
KNIGHT = CASSIA
ROOK = FLYING
LANCE = CHARIOT
PAWN = FOOT

# Upgraded = Promotion, Playing = is piece in play or not
NORMAL = 0
UPGRADED = 1
PLAYING = 1

SENTE = 0
GOTE = 1

BLACK = SENTE
WHITE = GOTE

piece_map = {
    'p': PAWN,
    'l': LANCE,
    'n': KNIGHT,
    's': SILVER,
    'g': GOLD,
    'b': BISHOP,
    'r': ROOK,
    'P': PAWN,
    'L': LANCE,
    'N': KNIGHT,
    'S': SILVER,
    'G': GOLD,
    'B': BISHOP,
    'R': ROOK
}


class GameSampler:
    '''
    Provides differnt ways to create sample and testing data from the set of
    games in particular file
    '''
    def __init__(self, file_name):
        self.source = file_name

    def single_random(self, N=10000, seed=False):
        '''
        Select a single move at random from N total games. The sample selects in the order
        of the first N games in the file.
        '''
        if seed:
            random.seed(seed)

        games = open(self.source, "r")
        result = []

        num_selected = 0
        select_index, move_index = 0, 0
        while num_selected < N:
            line = tuple(games.readline().strip().split(','))
            # See if we are at the start of a new game
            if len(line) == 1:
                # Reset our bookeeping for random selection, randomly select a move index
                move_index = 0
                select_index = random.randint(0, int(line[0].split(' ')[1]))
                continue

            # Get the pos, move pair
            if move_index == select_index:
                result.append(line)
                num_selected += 1

            move_index += 1

        return result

    def single_random_random(self, N=10000, seed=False):
        '''
        Select a single move at random from a N randomly selected games. Thus, N must be
        less than the total number of games in the file.
        '''
        if seed:
            random.seed(seed)

        games = open(self.source, 'r')
        games_dict = dict()

        moves = []
        game_no, num_moves = 0, 0

        for line in games:
            line = tuple(line.strip().split(','))
            if len(line) == 1:
                # If arrived at a new game, add its dictionary entry
                if game_no > 0:
                    games_dict[(game_no, num_moves)] = moves
                    moves = []
                # Update vars to new game info
                game_info = line[0].split(' ')
                game_no, num_moves = int(game_info[0]), int(game_info[1])
            else:
                moves.append(line)
        # Add the last one
        games_dict[(game_no, num_moves)] = moves

        # Make select a random game and a random move
        result = dict()
        for i in range(N):
            game = random.choice(list(games_dict.keys()))
            moves = games_dict.pop(game)
            move = random.choice(moves)
            result[game] = move

        return list(result.values())


def save_obj(obj, name):
    with open('obj/' + name + '.pkl', 'wb') as f:
        pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)


def genGomakind(id, upgrade, player):
    '''
    Function coppied from easyShogiCpp package for integer representation of a position.

    @id       :  PAWN | BISHOP | KNIGHT | ROOK, ... ETC
    @updgrade :  UPGRADE | NORMAL | PLAYING
    @player   :  SENTE | GOTE

    '''
    return id + (upgrade * 8) + (player * 16)


def encodePosHex(sfen):
    '''
    Take a sfen and return the hex string encoding of the shogi position. The hex vector
    is of the following format:
        - first 80 chars [0-80] are piece_types ===> Code requires use this function: genGomakind(PIECE_TYPE, NORMAL/UPGRADE, SENTE/GOTE)
        - Next 16 chars [81-96] handle pieces in hand
            - First 8 bits are SENTE == Black. Order is f s n l b r k g
            - Next 8 bits are GOTE = White. same order
            - bits store the COUNT of each piece.
        - Last 2 bits are for round (turn) number
            - item at 97th position multiplied by 256 and added to round?
                - Some kind of round multiplier —> set to 0?
            - 98th bit added to round…
                - Set to round number in hex
    '''

    piece_dict = {
        'p': PAWN,
        'l': LANCE,
        'n': KNIGHT,
        's': SILVER,
        'g': GOLD,
        'k': KING,
        'r': ROOK,
        'b': BISHOP
    }

    board, turn, hand, move_num = sfen.split(' ')
    rows = board.split('/')

    # Convert sfen blank space format (int) x, to x * '.'
    reformated = []
    for row in rows:
        replacement = []
        promotion = False
        for ch in row:
            if promotion:
                replacement[-1] += ch
                promotion = False
            elif ch.isdigit():
                for i in range(int(ch)):
                    replacement.append('.')
            elif ch == '+':
                replacement.append(ch)
                promotion = True
            else:
                replacement.append(ch)
        reformated.append(replacement)

    # Read board in order hex code for cpp package expects, right->left, top->bottom
    columns = []
    for i in range(8, -1, -1):
        result = []
        for row in reformated:
            result.append(row[i])
        columns.append(result)

    # Convert to integer representation
    int_rep = []
    for col in columns:
        new_col = []
        for piece in col:
            # Determine who piece belongs to
            player = BLACK if piece.isupper() else WHITE
            piece = piece.lower()
            # Check if upgraded, generate encodings
            if len(piece) > 1:
                piece_type = genGomakind(piece_dict[piece[1]], UPGRADED,
                                         player)
            elif piece == '.':
                piece_type = 255
            else:
                piece_type = genGomakind(piece_dict[piece], NORMAL, player)
            new_col.append(piece_type)
        int_rep.append(new_col)

    # Convert to hex then hex string
    hex_rep = [[hex(val)[2:].upper().zfill(2) for val in col]
               for col in int_rep]

    board_hex = ""
    for col in hex_rep:
        board_hex += "".join(col)

    # Get a dictionary of the players hand
    black_hand = dict()
    white_hand = dict()

    counts = re.findall(r'\d+', hand)
    captured = re.findall(r'[a-zA-Z]', hand)
    for i, piece in enumerate(captured):
        if piece.isupper():
            black_hand[piece.lower()] = int(counts[i])
        else:
            white_hand[piece.lower()] = int(counts[i])

    # Encode the hand in order expected by cpp packagd: p s n l b r k g
    ordering = ['p', 's', 'n', 'l', 'r', 'b', 'k', 'g']

    hand_hex = ""
    for piece in ordering:
        if piece not in black_hand:
            hand_hex += "00"
        else:
            hand_hex += hex(black_hand[piece])[2:].upper().zfill(2)

    for piece in ordering:
        if piece not in white_hand:
            hand_hex += "00"
        else:
            hand_hex += hex(white_hand[piece])[2:].upper().zfill(2)

    # Encode the round... -1 bc cpp has 0 indexed turns
    round_hex = hex((int(move_num) - 1) // 256)[2:].upper().zfill(2) + \
        hex((int(move_num) - 1) % 256)[2:].upper().zfill(2)

    # Combine the three portions of the encoding
    return board_hex + hand_hex + round_hex


def genMove(prePos, newPos, upgrade, playing):
    # playing means dropping a piece
    # pos are ints from 0-80 on the board
    # upgrade is NORMAL = 0 or UPGRADE = 1
    return (prePos * 81 + newPos) * 4 + upgrade * 2 + playing


def genPos(suji, dan):
    # Dan is row 1-9
    # Suji is col 1-9
    # Japanese notation, (1,1) is top right corner

    return -1 if (suji > 9 or suji < 1 or dan > 9
                  or dan < 1) else (suji - 1) * 9 + (dan - 1)


def usi_to_int(usi):
    # convert a move in usi representation to the int that the cppmodule expects
    numeric = {
        'a': 1,
        'b': 2,
        'c': 3,
        'd': 4,
        'e': 5,
        'f': 6,
        'g': 7,
        'h': 8,
        'i': 9
    }

    # Drop move
    if '*' in usi:
        piece = piece_map[usi[0]]
        newPos = genPos(int(usi[2]), numeric[usi[3]])

        return genMove(piece, newPos, NORMAL, PLAYING)

    # normal move
    else:
        prePos = genPos(int(usi[0]), numeric[usi[1]])
        newPos = genPos(int(usi[2]), numeric[usi[3]])
        upgrade = UPGRADED if '+' in usi else NORMAL

        return genMove(prePos, newPos, upgrade, NORMAL)


# not really for sampling, just scratch function used once
def print_castle_dict_as_cpp_map(reverse):
    def invert(sfen):
        '''
        Given a sfen representing a position from black's perspective, return the inversion.
        '''
        sfen = sfen.split(' ')
        inversion = sfen[0][::-1].lower()
        sfen[0] = inversion
        sfen[1] = 'b' if sfen[1] == 'w' else 'w'
        return ' '.join(sfen)

    castles = {
        "left_mino": "9/9/9/9/9/P1P6/1PBPP4/1KS1G4/LN1G5 b - 0",
        "gold_fortress": "9/9/9/9/9/2PPP4/PPSG5/1KG6/LN7 b - 0",
        "helmet": "9/9/9/9/9/P1P6/1PSPP4/2GKG4/LN7 b - 0",
        "crab": "9/9/9/9/9/2P1P4/PP1P5/2GSG4/LK1K5 b - 0",
        "bonanza": "9/9/9/9/9/2P6/PPSPP4/2KGG4/LN7 b - 0",
        "snowroof": "9/9/9/9/9/2PPP4/PP1SS4/2G1G4/LN1K5 b - 0",
        "silver_horns_snowroof": "9/9/9/9/9/2PP1PP2/PP1SPS3/2G1G4/LN1K5 b - 0",
        "right_king_1": "9/9/9/9/7P1/4PPP1P/5SN2/6K2/7RL b - 0",
        "right_king_2": "9/9/9/9/7PP/4PPP2/5SN2/5GK2/7RL b - 0",
        "right_king_3": "9/9/9/9/7P1/5PP1P/4PSN2/4GK3/7RL b - 0",
        "central_house": "9/9/9/9/9/9/2PPPPP2/2GSKSG2/9 b - 0",
        "nakahara": "9/9/9/9/9/2P6/PP1PPP3/2G2S3/LNSKG4 b - 0",
        "duck": "9/9/9/9/9/9/2PPPPP2/3SKS3/2G3G2 b - 0",
        "paperweight": "9/9/9/9/9/P1P1P4/1PNP5/LSKGG4/9 b - 0",
        "truck": "9/9/9/9/9/2PPPP3/1P1SS4/2KGG4/9 b - 0",
        "boat_pawn": "9/9/9/9/9/P1P1P4/1P1P5/1BK1G4/LNSG5 b - 0",
        "daughter_inside_box": "9/9/9/9/9/P1P6/1P1P5/1BKG5/LNSG5 b - 0",
        "diamond": "9/9/9/9/9/P1P1P4/1P1PS4/1BKSG4/LN1G5 b - 0",
        "strawberry": "9/9/9/9/9/P1P6/1P1PP4/1BGKG4/LNS6 b - 0",
        "yonenaga": "9/9/9/9/9/PPPPP4/1S1G5/KBG6/LN7 b - 0",
        "elmo": "9/9/9/9/9/P1P6/1P1P5/1BKS5/LNG6 b - 0",
        "elmo_gold": "9/9/9/9/9/P1P6/1P1P5/1BKS5/LNG1G4 b - 0",
        "silver_elephant_eye": "9/9/9/9/9/1PPPP4/2NKS4/3S5/9 b - 0",
        "gold_elephant_eye": "9/9/9/9/9/1PPPP4/2NKS4/3G5/9 b - 0",
        "kushikatsu": "9/9/9/9/9/2P6/PP1P5/KSG6/LNG6 b - 0",
        "anaguma": "9/9/9/9/9/9/5PPPP/6GSL/6GNK b - 0",
        "mino": "9/9/9/9/9/8P/4PPPP1/4G1SK1/5G1NL b - 0",
        "silver_crown": "9/9/9/9/9/5PPPP/5GNS1/6GK1/8L b - 0",
        "wall": "9/9/9/9/9/9/4PPPPP/5SK2/5G1NL b - 0",
        "gold_mino": "9/9/9/9/9/9/4PPPPP/5SGK1/7NL b - 0",
        "three_move": "9/9/9/9/9/9/4PPPPP/5GK2/6SNL b - 0",
        "rapid_castle": "9/9/9/9/9/9/5PPPP/6GK1/6SNL b - 0",
        "flatfish": "9/9/9/9/9/8P/4PPPP1/6SK1/4GG1NL b - 0",
        "millenium_1": "9/9/9/9/9/2PPP4/PPNG5/2G6/LKS6 b - 0",
        "millenium_2": "9/9/9/9/9/2PPP4/PPNG5/1SG6/LKS6 b - 0",
        "millenium_3": "9/9/9/9/9/PPPPP4/2NG5/1SG6/LKS6 b - 0",
        "millenium_4": "9/9/9/9/9/1PPPP4/PSNG5/2G6/LKS6 b - 0",
        "millenium_5": "9/9/9/9/9/2P6/PPNP5/1SGS5/LKG6 b - 0",
        "millenium_6": "9/9/9/9/9/1PPPP4/P2GS4/1BGS5/LNK6 b - 0",
        "gold_excelsior": "9/9/9/9/9/8P/4PPPP1/4GGKS1/7NL b - 0",
        "aerokin": "9/9/9/9/9/5PPPP/5GNSK/6G1L/9 b - 0",
        "aerial_tower": "9/9/9/9/PPP6/1KS6/1GN6/9/L8 b - 0",
        "fourth_edge_king": "9/9/9/9/PPPP5/KGGS5/2N6/9/L8 b - 0"
    }

    if (not reverse):
        for key, val in castles.items():
            print("{{\"{}\", {{\"{}\"}}}},".format(key, encodePosHex(val)))
    else:
        for key, val in castles.items():
            print("{{\"{}\", {{\"{}\"}}}},".format(key + "_white",
                                                   encodePosHex(invert(val))))


if __name__ == "__main__":

    # # Test for compatability with cpp package
    # import shogi
    # test1 = "l2l5/1ks1p4/2g3+R2/1pn1K1b2/p1pP2+bs1/8l/PPSN5/LSGG5/KN2b4 w 1P1G 10"
    # test2 = "ln1gkg1nl/1r1s1s1b1/p1pp1p1pp/1p2p1p2/9/2PPP4/PP3PPPP/1B1S1S1R1/LN1GKG1NL b - 11"

    # test2 = "l2l5/1ks1p4/2g3+R2/1pn1N1b2/p1pP2+bs1/8l/PPSN5/LSGG5/KN2r4 w 1G1P10p 126"
    # test2 = "+L6nl/1k5r1/1pn1S1b2/2P1s1p1p/3p1p1p1/4b1P1P/1P2+pP1P1/1Sp1p3R/KN1G3NL w 1G1L2P2g1s1p 100"

    # hex1 = encodePosHex(test2)
    # hex2 = encodePosHex(test2)

    # board1 = shogi.Board(test2)
    # board2 = shogi.Board(test2)

    # int_moves = []
    # for move in board1.legal_moves:
    #     int_moves.append(usi_to_int(move.usi()))

    # for move in sorted(int_moves):
    #     print(move)
    # print(board2.move_number)
    # print(board2)
    # print(hex2)
    # # CPP comptability testing stuff ends here

    # MAIN code for main running python3 sample.py ...
    parser = argparse.ArgumentParser(
        description=
        'Sample games and generate all legal moves from sampled positions')
    parser.add_argument("--fin",
                        type=str,
                        help="File in, the file of games to read from")
    parser.add_argument(
        "--n",
        type=int,
        help="Sample size, total number of games to use for training & testing."
    )
    parser.add_argument(
        "--method",
        type=str,
        help=
        "Selection method, either: sr (single random) or srr (single random random)"
    )
    parser.add_argument(
        "--cache",
        type=str,
        help=
        "Desired filename store the cache of legal moves reachable from all sampled positions: NO '/' allowed."
    )
    parser.add_argument(
        "--testOut",
        type=str,
        help="Desired filename storing the test positions: NO '/' allowed.")
    parser.add_argument(
        "--trainOut",
        type=str,
        help="Desired filename to store the train positions: NO '/' allowed.")
    parser.add_argument("--type",
                        type=str,
                        help="Output file type. Supports [pickle, json]")
    parser.add_argument(
        "--seed",
        type=int,
        default=None,
        help=
        "Seed for the random selection algorithm. Not necessary, but usefull for testing."
    )
    parser.add_argument(
        "--hex",
        type=bool,
        default=False,
        help=
        "[True | False] - Whether or not to convert positions from sfen to hex string representaiton."
    )
    parser.add_argument(
        "--includeResultPos",
        type=bool,
        default=False,
        help=
        "[True | False] - Whether or not to include representation of resulting move in the cache. False by default."
    )
    parser.add_argument("--move",
                        type=str,
                        default='usi',
                        help="Move representation, either 'usi' or 'int'")
    args = parser.parse_args()

    gp = GameSampler(args.fin)

    print("------------- Making '{}' Selection ---------------".format(
        args.method))

    selection = []
    if args.method == "sr":
        selection = gp.single_random(args.n, seed=args.seed)
    elif args.method == "srr":
        selection = gp.single_random_random(args.n, seed=args.seed)
    else:
        exit(0)  # Failed parsing

    # Some moves actually recorded into the website incorrectly, double check
    for i, game in enumerate(selection):
        pos, move = game[0], game[1]
        board = shogi.Board(pos)
        move = shogi.Move.from_usi(move)

        if move not in board.legal_moves:
            print("Invalid selection {}, {}".format(pos, move))
            print("Replacing . . .")
            valid = False
            while not valid:
                replace = gp.single_random_random(1, seed=args.seed)
                r_pos = replace[0][0]
                r_move = replace[0][1]
                r_board = shogi.Board(r_pos)
                r_move = shogi.Move.from_usi(r_move)
                if r_move in r_board.legal_moves:
                    selection[i] = replace[0]
                    print("Replaced invalid with {}, {}".format(
                        r_pos, r_move.usi()))
                    valid = True

    train = selection[:args.n // 2]
    test = selection[args.n // 2:]

    print("Selected {} moves for training, {} for testing".format(
        len(train), len(test)))

    import shogi

    print("--------------- Caching Legal Moves ----------------")
    widgets = [
        ' [',
        progressbar.Timer(), '] ',
        progressbar.Bar(), ' (',
        progressbar.AdaptiveETA(), ') ',
        progressbar.Percentage()
    ]
    bar = progressbar.ProgressBar(maxval=len(selection), widgets=widgets)
    completed_games = 0
    total_positions = 0

    # Cache to store all of the legal moves for all the N sample positions
    legal_moves_cache = dict()
    for game in selection:
        pos, move = game[0], game[1]
        board = shogi.Board(pos)

        actions = []
        for move in board.legal_moves:
            # Make the move and record the str representation of the new position=
            board.push(move)
            resulting_pos_str = board.sfen()

            if args.move == 'usi':
                move = move.usi()
            elif args.move == 'int':
                move = usi_to_int(move.usi())

            # Determine if we want to save the representation of resulting pos or just move
            action = (move, '')
            if args.includeResultPos:

                # Store usi move and a new board object of the resulting position
                if args.hex:
                    # Hex string representation
                    action = (move, encodePosHex(resulting_pos_str))
                else:
                    # Standard python shogi board object representation
                    action = (move, shogi.Board(resulting_pos_str))

            actions.append(action)

            # Reset the board state
            board.pop()

        # Add all of the move, resulitng board pairs to the dictionary
        if args.hex:
            legal_moves_cache[encodePosHex(pos)] = actions
        else:
            legal_moves_cache[pos] = actions

        # Update the progressbar and bookkeeping
        bar.update(completed_games)
        completed_games += 1
        total_positions += len(actions)

    bar.finish()

    dest = "/json"
    if args.type == "pickle":
        dest = "/obj"

    print("-------------- Saving Caches to {} ---------------".format(dest))

    # Last minute conversiion of move types if requested int not usi
    if args.move == 'int':
        train = [(game[0], usi_to_int(game[1])) for game in train]
        test = [(game[0], usi_to_int(game[1])) for game in test]

    # Last minute convert to hex for the train and test data
    if args.hex:
        train = [(encodePosHex(game[0]), game[1]) for game in train]
        test = [(encodePosHex(game[0]), game[1]) for game in test]

    if args.type == "pickle":
        save_obj(legal_moves_cache, args.out)
        save_obj(train, args.trainOut)
        save_obj(test, args.testOut)

    elif args.type == "json":

        json_cache = []
        json_train = []
        json_tests = []
        for board, actions in legal_moves_cache.items():

            # Legal moves cache
            cache_entry = dict()
            cache_entry["board"] = board
            cache_entry["actions"] = actions

            json_cache.append(cache_entry)

        for action in train:
            board, move = action[0], action[1]

            # Train board and professional move pair
            train_entry = dict()
            train_entry["board"] = board
            train_entry["pmove"] = move

            json_train.append(train_entry)

        for action in test:
            board, move = action[0], action[1]

            tests_entry = dict()
            tests_entry["board"] = board
            tests_entry["pmove"] = move

            json_tests.append(tests_entry)

        # Save them to appropriate files
        with open('json/' + args.cache + '.json', 'w') as fp:
            json.dump(json_cache, fp)

        with open('json/' + args.trainOut + '.json', 'w') as fp:
            json.dump(json_train, fp)

        with open('json/' + args.testOut + '.json', 'w') as fp:
            json.dump(json_tests, fp)

    print("-------------- Statistics from Games ---------------")
    print("Games: {}".format(args.n))
    print("Positions: {}".format(total_positions))

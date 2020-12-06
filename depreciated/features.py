import shogi
import time
import random
import progressbar
from sample import GameSampler
from types import FunctionType

# shogi.PIECE_SYMBOLS
#   0   1    2 .............................................................. 14
# ['', 'p', 'l', 'n', 's', 'g',　'b', 'r', 'k', '+p', '+l', '+n', '+s', '+b', '+r']
# empty, pawn, lance, knight, silver, gold, bishop, rook, king


class ShogiFeatures:
    '''
    Wrapper class to for features of shogi games. The class methods represent abstract
    conditions of a shogi game such as King Safety, Good shape, bad shape, Material Possesion,
    etc. Each of these abstractions contains a set of one or more feature functions that all
    take the same input @pos, which is a shogi.Board object, and return some normalized value.
    '''
    def all(self):
        '''
        Return a list of references to ALL of the feature functions nested within the
        methods of this class
        '''
        features = []
        for method in dir(self):
            func = getattr(self, method)
            if type(func) == FunctionType:
                for feature in func():
                    features.append(feature)
        return features

    # ---------------------------- Abstract Shogi Conditions ----------------------------
    @staticmethod
    def material():
        '''
        Each feature returns the count of a given piece on the board available for use
        by the current player.
        '''
        def count(piece, board):
            '''
            Simply count the number of occurences of a given piece on the board
            '''
            piece = piece.upper(
            ) if board.turn == shogi.BLACK else piece.lower()
            return sum([
                1 for i in shogi.SQUARES
                if board.piece_at(i) and board.piece_at(i).symbol() == piece
            ])

        def create_counter(piece):
            '''
            Creates and returns a unique function that counts the number of a given piece on a board.
            * Yay for python closures! *
            '''
            def piece_counter(board):
                return count(piece, board)

            return piece_counter

        def piece_counters():
            '''
            Generate counters for all shogi pieces and promotions (except the king).
            '''
            pieces = [
                'p', 'l', 'n', 's', 'g', 'b', 'r', '+p', '+l', '+n', '+s',
                '+b', '+r'
            ]

            return [create_counter(piece) for piece in pieces]

        # End of material feature, return counter functions for all piece types!
        return piece_counters()

    @staticmethod
    def king_safety():
        '''
        Set of features that represent the overall safety of the king.
        '''
        def king_area(board):
            '''
            Helper function for all the features to get all of the valid squares surrouding
            the position of the current players king.
            '''
            king = 'K' if board.turn == shogi.BLACK else 'k'
            index = [
                i for i in shogi.SQUARES
                if board.piece_at(i) and board.piece_at(i).symbol() == king
            ]
            if index:  # Check as some board positions could have been endgame situations
                return list(
                    shogi.SquareSet(
                        board.attacks_from(shogi.KING, index[0],
                                           board.occupied, board.turn)))
            else:
                return []

        def thickness(board):
            '''
            Number of defending pieces immediately surrounding the king.
            '''
            defenders = 0
            for pos in king_area(board):
                piece = board.piece_at(pos)
                if piece:
                    # Look for uppercase pieces if black
                    if board.turn == shogi.BLACK and piece.symbol().isupper():
                        defenders += 1
                    # Look for uppercase pieces if white
                    elif board.turn == shogi.WHITE and piece.symbol().islower(
                    ):
                        defenders += 1
            return defenders

        def escape_routes(board):
            '''
            Number of empty spaces immediately surrounding the king.

            TO DO : Also make sure the spaces are not attacked directly by enemy (no suicide escapes)
            '''
            return sum(
                [1 for pos in king_area(board) if not board.piece_at(pos)])

        def threats(board):
            '''
            Total number of attackers targeting the squares around the king. If multiple of the
            opponent's pieces attack the same square, both of these instances are counted.
            '''
            opponent = board.turn ^ 1  # shogi.WHITE if board.turn == shogi.BLACK else shogi.BLACK
            return -1 * sum([
                len(board.attackers(opponent, pos)) for pos in king_area(board)
            ])

        def entering_king():
            pass

        return ([thickness, escape_routes, threats])

    @staticmethod
    def pieces_in_hand():
        #     '''
        #     Set of features that tries to represent the strategic potential of dropping
        #     pieces they have captured back onto the board.
        #     '''
        def in_hand_count(board):
            return len(board.pieces_in_hand[board.turn])

    #     # Uncomment for an alternative feature, but quite costly as it looks ahead one more level in game tree
    #     def max_drop_attack(board):
    #         '''
    #         Returns the maximum number of enemy pieces one could target by placeing any single
    #         captured piece back on any open board square.
    #         '''
    #         max_attack = 0
    #         pieces_in_hand = list(board.pieces_in_hand[board.turn].keys())
    #         for piece in pieces_in_hand:

    #             # Get all of the empty squares on the board
    #             p1 = set(shogi.SquareSet(board.occupied[board.turn]))
    #             p2 = set(shogi.SquareSet(board.occupied[board.turn ^ 1]))
    #             occupied = p1.union(p2)
    #             empty_squares = occupied.symmetric_difference(shogi.SQUARES)

    #             for square in empty_squares: # For all the empty squares
    #                 # Create a drop move that places the piece in hand on a square
    #                 move = shogi.Move(None, square, drop_piece_type=piece)
    #                 if board.is_legal(move):
    #                     # Actually make the move if it is valid
    #                     board.push(move)
    #                     # See how many enemies the dropped piece attacks
    #                     attacks = shogi.SquareSet(board.attacks_from(piece, square, board.occupied, board.turn ^ 1))
    #                     # Tally the squares that actually have an enemy in there
    #                     attack_total = 0
    #                     for pos in attacks:
    #                         target = board.piece_at(pos)
    #                         if target and target.color != board.turn ^ 1:
    #                             attack_total += 1

    #                     # Restore the state of the board, update best seen
    #                     board.pop()
    #                     max_attack = max([max_attack, attack_total])

    #         return max_attack

    # Return our list of feature functions

        return ([in_hand_count])

    @staticmethod
    def controlled_squares():
        '''
        Represents a way to measure the "freedom" of a player's movment. More freedom of movment means
        more options and more chances for lasting advantages. It is most important to control the squares
        in one's own camp (closet 3 ranks [rows] to your side), but some weight should also be given if a square
        in the enemy camp is more attcked than defended.
        '''
        def camps(board):
            '''
            Helper function which returns a tuple of lists representing the squares of the current player's
            camp as well as the enemy camps.
            '''
            home_camp = list(range(
                0, 27)) if board.turn == shogi.WHITE else list(range(54, 81))
            oppn_camp = list(range(
                0, 27)) if board.turn == shogi.BLACK else list(range(54, 81))
            opponent = board.turn ^ 1  # shogi.WHITE if board.turn == shogi.BLACK else shogi.BLACK
            return (home_camp, oppn_camp, opponent)

        def in_camp_freedom(board):
            '''
            Counts the number of squares within the curent player's camp (closet 3 ranks) which are
            safe. Safe squares are those which are both unnocupied and not currently in a direct line
            of attack by an opponent's piece.
            '''
            home_camp, oppn_camp, opponent = camps(board)

            free = 0
            for square in home_camp:
                piece = board.piece_at(square)
                attackers = board.attackers(opponent, square)
                # Ensure square is empty AND not attacked
                if not piece and not attackers:
                    free += 1

            return free

        def out_camp_attack(board):
            '''
            Counts the number of squares in the opponent's camp which are more attacked than defended.
            '''
            home_camp, enemy_camp, opponent = camps(board)

            vulnerable = 0
            for square in enemy_camp:
                piece = board.piece_at(square)
                # Ensure it is non-empty and has an opponents piece
                if piece and piece.color == opponent:
                    # Count the number of attackers from current players perspective
                    attackers = len(board.attackers(board.turn, square))
                    # Number of defenders is just number of attackers from opponents perspective
                    defenders = len(board.attackers(opponent, square))

                    if attackers > defenders:
                        vulnerable += 1

            return vulnerable

        return ([in_camp_freedom, out_camp_attack])

    @staticmethod
    def castle():
        '''
        Tries to capture how well-formed a given player's castle structure is.
        '''
        # Threshold at which we consider a position to contain a proper castle.
        #   A player's formation must have at least as many pieces (excluding the king) in the
        #   proper position as the total number of pieces in the castle - CASTLE_THRESHOLD. The
        #   king must ALWAYS be in the correct position to be considered a castle.
        #
        #   ex. For a threshold of 1, if a player's king is in the correct spot for the castle,
        #       a maximum of at most 1 other piece is allowed to be absent/different from the
        #       full/proper formation.

        CASTLE_THRESHOLD = 2

        # Standard castles BLACK perspective; must call invert() to get WHITE perspective
        castles = {
            "left_mino": "9/9/9/9/9/P1P6/1PBPP4/1KS1G4/LN1G5 b - 0",
            "gold_fortress": "9/9/9/9/9/2PPP4/PPSG5/1KG6/LN7 b - 0",
            "helmet": "9/9/9/9/9/P1P6/1PSPP4/2GKG4/LN7 b - 0",
            "crab": "9/9/9/9/9/2P1P4/PP1P5/2GSG4/LN1K5 b - 0",
            "bonanza": "9/9/9/9/9/2P6/PPSPP4/2KGG4/LN7 b - 0",
            "snowroof": "9/9/9/9/9/2PPP4/PP1SS4/2G1G4/LN1K5 b - 0",
            "silver_horns_snowroof":
            "9/9/9/9/9/2PP1PP2/PP1SPS3/2G1G4/LN1K5 b - 0",
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

        def invert(sfen):
            '''
            Given a sfen representing a position from black's perspective, return the inversion.
            '''
            sfen = sfen.split(' ')
            inversion = sfen[0][::-1].lower()
            sfen[0] = inversion
            sfen[1] = 'b' if sfen[1] == 'w' else 'w'
            return ' '.join(sfen)

        def in_castle_threshold(board):
            '''
            Return 1 if we are within the threshold of at least one proper castle formation.
            '''
            for sfen in castles.values():
                # Set the proper peerspective for the castle
                sfen = sfen if board.turn == shogi.BLACK else invert(sfen)
                # Initialize the castle
                castle = shogi.Board(sfen)
                # Transform the castle to a square set to get square indicies
                castle_squares = shogi.SquareSet(castle.occupied[board.turn])

                incorrect = 0
                king_in_place = True
                for square in castle_squares:
                    # Get the piece at the corresponding square in the player's board and castle board
                    p_piece = board.piece_at(square)
                    c_piece = castle.piece_at(square)

                    if not p_piece:  # Empty square
                        incorrect += 1
                    elif p_piece.color != board.turn:  # Enemy piece
                        incorrect += 1
                    elif p_piece.piece_type != c_piece.piece_type:  # Wrong piece
                        if c_piece.piece_type == shogi.KING:  # Player's King in wrong place
                            king_in_place = False
                        else:
                            incorrect += 1

                if king_in_place and incorrect <= CASTLE_THRESHOLD:
                    return 1

            # If we made it through all castle formations without finding a match, return 0
            return 0

        return ([in_castle_threshold])

    @staticmethod
    def board_shape():
        '''
        As of now a set of two simple features that return a bools of whether or not a give position
        is a "good" or "bad" shape. These board shapes are arbitrary but have been chosen from a
        collection of games with substantial professional commentary.
        '''
        def find_pieces(board, piece_type):
            '''
            Helper function returns the board index of a the shogi piece/s of type @piece_type
            that belong to the current player. @piece_type is based on shogi.PIECE_SYMBOLS
            '''
            symbol = shogi.PIECE_SYMBOLS[piece_type]
            piece = symbol.upper() if board.turn == shogi.BLACK else symbol
            indecies = [
                i for i in shogi.SQUARES
                if board.piece_at(i) and board.piece_at(i).symbol() == piece
            ]
            return indecies

        # -------------------------------- Features of BAD positions --------------------------------
        def gold_ahead_silver(board):
            '''
            It is generally considered a disadvantage to have a gold piece directly in front of a
            silver piece since the silver piece cannot move one square backwards, thus leaving an
            oppening for an opponent. It is important to note that in some situations, such a
            formation could actually be advantageous, but that consideration is left out here
            '''
            gold_indecies = find_pieces(board, shogi.GOLD)
            if not gold_indecies:
                return 0

            for index in gold_indecies:
                index_behind = index + 9 if board.turn == shogi.BLACK else index - 9

                # Make sure we do not fall of the board
                if 0 <= index_behind and index_behind < 81:
                    # Return true if we actually have a gold ahead of silver formation
                    piece = board.piece_at(index_behind)
                    if piece and piece.color == board.turn and piece.piece_type == shogi.SILVER:
                        return -1

            # Did not find stack gold/silver, return false
            return 0

        def gold_adjacent_rook(board):
            '''
            It is generally considered 'bad shape' to have a gold directly adjacent to a rook. This is
            because not only does the gold restrict the rook's side to side movement, the gold piece
            also attackes the square ahead of the rook, which is an unecessary defense since the rook
            attacks all squares ahead of it anyways.
            '''
            rook_index = find_pieces(board, shogi.ROOK)
            if not rook_index:  # Ensure a rook is in play
                return 0

            left = rook_index[0] - 1
            right = rook_index[0] + 1

            adjacent_squares = [rook_index[0] - 1, rook_index[0] + 1]
            for square in adjacent_squares:
                if 0 <= square and square < 81:
                    piece = board.piece_at(square)
                    if piece and piece.color == board.turn and piece.piece_type == shogi.GOLD:
                        return -1

            return 0

        def boxed_in_bishop(board):
            '''
            Return 1 if the current player has their bishop completely locked in by their own pieces.
            '''
            MOVE_BUFFER = 10  # have a move buffer since bishops begin already boxed in

            bishop = find_pieces(board, shogi.BISHOP)
            if not bishop:  # Insure bishop in play
                return 0

            bishop = bishop[0]
            corners = [bishop - 10, bishop - 8, bishop + 8, bishop + 10]

            trapped = True
            for corner in corners:
                if 0 <= corner and corner < 81 and not board.piece_at(corner):
                    trapped = False

            return -1 if trapped and board.move_number - MOVE_BUFFER > 0 else 0

        def piece_ahead_of_pawns(board):
            '''
            Return the number of the current player's pieces that are at least BUFFER squares
            ahead of the pawn on the same file.
            '''
            SPACE_BUFFER = 2  # Set so that feature is mutually exclusive with reclining_silver()
            RANKS_FORWARD = 3  # How many ranks forward to search, avoids penalizing late game formations

            pawns = find_pieces(board, shogi.PAWN)
            if not pawns:
                return 0

            ahead_of_pawn_count = 0
            iterator = -9 if board.turn == shogi.BLACK else 9
            for pawn in pawns:

                # Start check at SPACE_BUFFER squares in front of the pawn.
                index = pawn + (SPACE_BUFFER * iterator)
                ranks_to_search = RANKS_FORWARD

                # Check all the remaining squares for friendly pieces
                while 0 <= index and index < 81 and ranks_to_search > 0:
                    piece = board.piece_at(index)
                    if piece and piece.color == board.turn:
                        ahead_of_pawn_count += 1

                    # Move forward in the file (up or down depending on perspective)
                    index += iterator
                    ranks_to_search -= 1

            return -1 * ahead_of_pawn_count

        # -------------------------------- Features of GOOD positions -------------------------------
        def bishop_head_protected(board):
            '''
            Since a bishop cannot move directly forward, it is generally a 'good shape' to have the
            square in front of the rook be protected,
            '''
            bishop_index = find_pieces(board, shogi.BISHOP)
            if not bishop_index:  # Ensure bishop in play
                return 0

            bishop_index = bishop_index[0]
            head = bishop_index - 9 if board.turn == shogi.BLACK else bishop_index + 9

            defenders = 0
            if 0 <= head and head < 81:
                defenders = len(board.attackers(board.turn, head))

            return 1 if defenders > 0 else 0

        def reclining_silver(board):
            '''
            Return 1 if there is a "reclining silver" 腰掛け銀 shape on the board. Gets
            this name because it looks like the silver is sitting in a chair of pawns.

            The shape is something like:      S P
                                              P

                                     or:    P S
                                              P
            '''
            silvers = find_pieces(board, shogi.SILVER)
            if not silvers:
                return 0

            for silver in silvers:

                sides = [silver + 1, silver - 1]

                # Only the bottom portion of the bench depends on player's turn
                leg = silver + 9 if board.turn == shogi.BLACK else silver - 9

                # Both formations must have a leg of the chair
                if 0 <= leg and leg < 81:
                    leg = board.piece_at(leg)
                    if not leg or leg.color != board.turn or leg.piece_type != shogi.PAWN:
                        continue
                else:
                    # Leg not in range, move to check next silver for a chair shape
                    continue

                # Now check for either left or right leaning chair
                for pos in sides:
                    if 0 <= pos and pos < 81:
                        piece = board.piece_at(pos)
                        if piece and piece.color == board.turn and piece.piece_type == shogi.PAWN:
                            return 1

            # Neither a left leaning nor a right leaning chair found
            return 0

        def claimed_files(board):
            '''
            A player is said to have 'claimed place in a file' if they have a pawn in the fifth rank
            that is defended from behind. This feature returns the number of files the current
            player has taken.
            '''

            fifth_rank = [36, 37, 38, 39, 40, 41, 42, 43, 44]
            claimed = 0

            for square in fifth_rank:
                piece = board.piece_at(square)
                if piece and piece.color == board.turn and piece.piece_type == shogi.PAWN:
                    # Check if the pawn is supported
                    if list(board.attackers(board.turn, square)):
                        claimed += 1

            return claimed

        def adjacent_silvers(board):
            '''
            Having two silvers directly next to eachoter is considered a strong position since they
            fill in eachother's gaps in backward movement. This feature returns 1 if adjacent silvers
            are present.
            '''
            silvers = find_pieces(board, shogi.SILVER)
            if len(silvers) < 2:
                return 0

            return 1 if abs(silvers[1] - silvers[0]) == 1 else 0

        def adjacent_golds(board):
            '''
            As with silvers, adjacent golds are strong. This feature reaturns 1 if adajent golds are present.
            '''
            golds = find_pieces(board, shogi.GOLD)
            if len(golds) < 2:
                return 0

            return 1 if abs(golds[1] - golds[0]) == 1 else 0

        return [
            gold_ahead_silver, gold_adjacent_rook, boxed_in_bishop,
            piece_ahead_of_pawns, bishop_head_protected, reclining_silver,
            claimed_files, adjacent_silvers, adjacent_golds
        ]


# Used when testing the functionality of features.py on its own
# def invert(sfen):
#     '''
#     Given a sfen representing a position from black's perspective, return the inversion.
#     '''
#     sfen = sfen.split(' ')
#     inversion = sfen[0][::-1].lower()
#     sfen[0] = inversion
#     sfen[1] = 'b' if sfen[1] == 'w' else 'w'
#     return ' '.join(sfen)

# import pickle
# def load_obj(name):
#     with open('obj/' + name + '.pkl', 'rb') as f:
#         return pickle.load(f)

# if __name__ == "__main__":
#     game_file = "games.txt"

#     # gp = GameSampler(game_file)
#     # selection = gp.single_random(1000, seed=64)
#     # train = selection[:1000//2]

#     legal_moves_cache = load_obj("legal_moves_cache")
#     TRAIN = load_obj("train_data")

#     # Selections that test specific 'good' / 'bad' shape features
#     # selection = [("l6nl/2gk3p1/2nsppb2/ppSp2R2/2S5p/P1pP5/1P2PP2P/2+B6/LNKGG2+rL w 1S2P1g1n2p 72", "rando")]
#     # selection = [("ln4snl/3g1kgb1/p1p1p2pp/1r1pspp2/9/2PPSP1R1/PP1SP1P1P/1BGKG4/LNS4NL b - 10", "reclining_silver")]
#     # selection = [("lnsgk1snl/1r4gb1/p1pppp1pp/6p2/1p5P1/2P6/PP1PPPP1P/1BG3GR1/LNS1K1SNL b - 10", "gold_adjacent_rook")]
#     # selection = [("9/9/9/9/9/2PP5/PPN6/1BG6/L1S6 b - 11", "boxed_in_bishop")]
#     # selection = [(invert("9/9/9/9/9/2PP5/PPNSS4/1BG6/L8 b - 11"), "adjacent_silvers")]
#     # selection = [(invert("9/9/9/9/9/2PP5/PPN6/1BGG5/L1S6 b - 11"), "adjacent_golds")]
#     # selection = [(invert("9/9/9/9/2P6/P1S1P2P1/1P1PSPP1P/1BK1G2R1/LN1G3NL b - 10"), "claimed_file")]

#     features = ShogiFeatures().all()
#     def H(board):
#         # sum([feature(board) for feature in features])
#         return [feature(board) for feature in features]

#     positions = 0
#     start = time.time()
#     for game in TRAIN:
#         pos, move = game[0], game[1]
#         for action in legal_moves_cache[pos]:
#             move, result_pos = action[0], action[1]

#             H(result_pos)
#             positions += 1

#     # start = time.time()
#     # for item in train:
#     #     pos, move = item[0], item[1]
#     #     board = shogi.Board(pos)
#     #     # print(board)
#     #     H(board)

#     print("Evaluated H() for {} positions, took {}s".format(positions, time.time() - start))

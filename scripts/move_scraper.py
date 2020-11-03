import re
import csv 
import json
import shogi.KIF
import requests
import progressbar
import datetime as dt
from bs4 import BeautifulSoup


class Db2GameParser:
    '''
    This class is used to parse the game data stored at shogidb2.com in different
    formats.
    '''
    def __init__(self, base_url):
        self.base_url = base_url

    @staticmethod
    def pull_data(game_url):
        '''
        Pull game data from the given game url for shogidb2.com and convert it 
        into a python object.
        '''
        # Request the html from the game webpage
        page = requests.get(game_url)
        soup = BeautifulSoup(page.text, 'html.parser')

        # Extract the data containing shogi moves
        pattern = re.compile(r'var data =.*')
        script = soup.find("script", text=pattern)

        # Remove the 'var data =' pretext and convert to python object
        data = script.text[10:len(script.text) - 1]
        data = json.loads(data)

        return data

    @staticmethod
    def create_kif_str(data):
        '''
        Given a dictonary of game data created from an entry in the shogidb2.com database,
        returns a string in the Japanese standardized "kif" format.
        '''
        # Get list of moves
        moves = list(map(lambda move: move["move"], data["moves"]))
        
        # Convert time to format KIF parser expects
        time = dt.datetime.strptime(data["開始日時"], "%Y-%m-%dT%H:%M:%S.%fZ").strftime("%Y/%m/%d %H:%M:%S") # Maybe make everythin +9 here for GMT Japan time
        tournament = data["棋戦"]
        loc = data["場所"]
        limit = data["time"]
        handicap = data["手合割"]
        first = data["先手"]
        second = data["後手"]
        strat = data["戦型"]
        result = data["result"]

        kif = "開始日時：{}\n"\
              "棋戦：{}\n"\
              "場所：{}\n"\
              "持ち時間：{}\n"\
              "手合割：{}\n"\
              "先手：{}\n"\
              "後手：{}\n"\
              "戦型：{}\n"\
              "手数----指手---------消費時間--\n"\
              "".format(time, tournament, loc, limit, 
                        handicap, first, second, strat)
        # Add all the moves
        for i, move in enumerate(moves):
            kif += "{} {}\n".format(i+1, move)
        # Find the winner
        winner = ""
        if result == "gote_win":
            winner = "後手"
        elif result == "sente_win":
            winner = "先手"
        else:
            winner = result
            # print("FOUND A TIE, format is: {}".format(result))
            # exit(-1)

        win_str = "まで{}手で{}の勝ち".format(len(moves)-1, winner)
        kif += win_str

        return kif

    def get_kif(self, url):
        '''
        Class user method. Both pulls the data from the url and converts
        it into a kif_string.
        '''
        return self.create_kif_str(self.pull_data(url))

    @staticmethod
    def create_pos_move_pairs(data, wins=True):
        '''
        Given dictonary of a single game's data taken from shogidb2.com, creates an array 
        of tuples representing in the following format: "pos, move". @win specifies whether
        or not we take only pairs for the winning player.

        Positions are represented by standard "sfen" format:
            ex. ln5nl/2kgr2g1/3s1s3/pppppbpRp/5N3/P1PPP1P1P/1P2S4/2KGGS3/LN6L b 1B2P2p 52

            - Each portion separated by '/', read left to right, represents a row on the board,
              top to bottom, with numbers indicating blank spages. White pieces are uppercase 
              and black lowercase. The next column is a single letter, 'w' or 'b' designating
              who is to make the next move. The third column represents the number and type of
              a captured piece. The last column is the move number in the game.

        Moves are represented by the standard USI format:
            ex. 4a5b+, 4a3b, 2d2b etc.

        Note: The data from shogidb2 is in a different, more difficult to read Japanese format.
              Thus, we use the python-shogi library to convert these to USI formatted moves
        '''
        # Get the sfens directly from the data
        sfens = list(map(lambda entry: entry["sfen"], data["moves"]))

        # Generate a standard format kif, parse it
        kif_str = Db2GameParser.create_kif_str(data)
        kif = shogi.KIF.Parser.parse_str(kif_str)[0]

        # Extract USI formatted moves from the parsed data
        moves = kif['moves']

        initial_sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1"
        pos_move_pairs = [(initial_sfen, moves[0])]
        for i in range(1, len(moves)):
            sfen = sfens[i-1]
            move = moves[i]
            pos_move_pairs.append((sfen, move))

        # Find out who won the game and see if we want just the winner's moves
        winner = kif['win']
        if wins:
            # Black goes first, so return odd number moves (1, 3, 5...)
            if winner == 'b':
                return pos_move_pairs[::2]
            # Black goes second, so return even num moves (2, 4, 6...)
            elif winner == 'w':
                return pos_move_pairs[1::2]
            # Otherwise regiester a tie/impass...write to file and figure out what to do later
            else:
                # with open("ties.txt", "w") as f:
                #     f.write("Found a non-win game:\n")
                #     f.write(kif_str)
                return []

    def get_game(self, url, wins=True):
        return self.create_pos_move_pairs(self.pull_data(url), wins)

    def log(self, message, console=True):
        if not console:
            with open("log.txt", "a") as f:
                f.write("[Db2GameParser {}] : {}\n".format(dt.datetime.now(), message))
        else:
            print("[Db2GameParser {}] : {}".format(dt.datetime.now(), message))

    def scrape_games(self, in_file, out_file):
        '''
        Main function in charge of scraping the game data for each of the
        game urls provided by @in_file. Each game is written to out_file
        int the follwing format:
            index game_length
            pos, move
            pos, move
            ...
        '''
        start_time = dt.datetime.now()
        NUM_GAMES = 59281 # Hardcoded length of file

        # Set up the progress bar and reset the log file
        widgets=[
                ' [', progressbar.Timer(), '] ',
                progressbar.Bar(),
                ' (', progressbar.AdaptiveETA(), ') ',
                progressbar.Percentage()
                ]
        bar = progressbar.ProgressBar(maxval=NUM_GAMES, \
            widgets=widgets)
        bar.start()
        open("log.txt", "w").close()

        # Initialize constants
        ties = 0
        errors = 0
        valid_games = 0
        total_moves = 0
        game_num = 1
        with open(in_file, 'r') as fin, open(out_file, 'w') as out:
            # Read all of the games
            for path in fin:
                game_url = self.base_url + path.strip()

                try:
                    # Get the (pos, move) pairs for the winner of the game
                    game = self.get_game(game_url)                        
                    if game: 
                        game = list(map(lambda entry: entry[0] + ',' + entry[1] + '\n', game))
                        header = "{} {}\n".format(game_num, len(game))

                        # Write the game data
                        out.write(header)
                        out.write(''.join(game))

                        self.log("Copied {} moves from game {}".format(len(game), game_num), False)
                        valid_games += 1
                        total_moves += len(game)

                    # Skip if we found a tie or stalemate game
                    else:
                        self.log("Found a non-win outcome for game {}".format(game_num), False)
                        ties += 1
            
                except Exception as e:
                    self.log("Something went wrong for game {}".format(game_num), False)
                    self.log(" ---> url is: {}".format(game_url), False)
                    self.log(str(e), False)
                    errors += 1

                # Move to the next game 
                bar.update(game_num)
                game_num += 1

        # Append some statistics to the end of the log file
        logger = open("log.txt", "a")
        logger.write("\n---------- STATS ----------\n"\
                     "num games........: {}\n"\
                     "num ties.........: {}\n"\
                     "errors...........: {}\n"\
                     "valid games......: {}\n"\
                     "total positions..: {}\n"\
                     "---------------------------\n".format(game_num - 1, ties,
                                                            errors, valid_games,
                                                            total_moves))
        logger.write("took: {}".format(dt.datetime.now() - start_time))
        logger.close()
        # bar.finish()
                                              


if __name__ == "__main__":

    base_url = "https://shogidb2.com"
    parser = Db2GameParser(base_url)

    # Big boy, run them all!
    parser.scrape_games(in_file="./data/game_urls.txt", out_file="../games.txt")

    # Test file
    # parser.scrape_games(in_file="game_urls_test.txt", out_file="games_test.txt")

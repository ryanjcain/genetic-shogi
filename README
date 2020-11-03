# GA for Shogi

## Pre-Setup
1) Remove the `'-9'` option from the CXX flags option in the makefile if your system does not support it. (The code runs perfectly fine with `g++` alone on the Zoo)
Everything should work after unzipping except one change **MUST** be made to the file `features.hpp`.

Navigate to `/cpp` and change the */MY/ABSOLUTE/PATH* portion of the variables `lmcache` and `train_in` at the top of the file to the absolute path of wherever the project file has been unzipped. If you try to run and later on get some error message about json not parsing, it is because the paths here are incorrect. 
  - I did not quite have time to make a clean work around for this, but basically here we are setting the path to the json file that stores the cache of legal moves and the list of labeled training data. For some reason the json C++ package I am using complains if it does not have the absolute path.

## Running
1) Navigate to `/cpp` and run `make`. (By default this makes the shared library `shogilib.so` that gets used by python).
2) Go back to the main directory and activate the python virtual environment with: `. venv/bin/activate`
4) Run `python3 evolve.py`

Take a look at `log.txt` to see the results of the execution. Or since execution can take awhile even for 500 training positions, you can periodically check log.txt to see the progress of the GA. Note that the min and max values represent the *square* of the total number of positions guessed correctly.

*NOTE :* If you are having import errors there could be something wrong with the python venv. If that is the case, I have included a list of the dependancies I have successfully installed on the zoo. This is contained in `requirements.txt` and to setup a new venv this way run: `python3 -m venv venv` in the shogi directory, activate the venv again, then run `pip install -r requirements.txt`

## Settings and Config
- The main `evolve.py` script should print out all of the default GA settings / configuration for the algorithm, but these can very easily be changed at the top of `config.py`. They should be fairly self explanetory.

- By default there is already a loaded selection of 500 training moves in the the json directory and evolve.py is set to run the C++ version of evaluation with a very small population size. A larger sample of training data can be generated with `sample.py` (The paper used a size N=10,000 split in half between training and testing. See other options for details on how to run this script).
## Other Options
- Getting a larger sample size for training data. The script `sample.py` has what I think is a detailed enough arg parser that you can figure out what parameters to provide. However, be careful with the name and path of the in and out files. A correct path to the infile is necessary and right now both the cpp and python versoin of evolve funtion expect the cache file to have the name `legal_moves_cache`. The labeling formats also differ depending on if you are running cpp version or python.Here are the commands I used to generate the current set of 500 training positions for both the python and cpp versions.
  - **CPP:** `python3 sample.py --fin data/games_small.txt --out legal_moves_cache --method sr --n 1000 --type json --seed 64 --hex True --move int`
  - **Python:** `python3 sample.py --fin data/games_small.txt --out legal_moves_cache --method sr --n 1000 --type pickle --seed 64`
  - Lastly, depending on the size of the game file, sampling can take quite some time which is why I have just been working with `/data/games_small` which contains about 1000 games. I did not include my entire database file of over 60,000 games but can zip and send it over if you would like. It is also quite time consuming since I figured out pretty late that a surprising amount of the moves I collected from online are *invalid*. There are several typos, especially in games that were sampled from many years ago as they were inputted by hand I presume. Thus I have to additoinally check if a selected move is legal, which takes time with python-shogi. But again this only really needs to be done once and I have larger cache files saved on my machine already, 

- There are some other options for make　too, mainly `featuresTest` which will run the main() from features.cpp. There are some comented lines of code in the `evaluate_organism` function that output the time it took for an individual organism evaluation that can be interesting to take a look at and compare to python.
  - Note: *MUST* remove `-fPIC -shared` from `CXXFLAGS` for this to work.
- If you are playing around with `sample.py` and are selecting new training data / relabeling it, by default it saves in an ugly format. Run `make lmcache-pretty` and go into lmcache.cpp and uncomment main() to save a more human readable copy of legal moves cache / training data to take a look at.
  - Note: *MUST* remove `-fPIC -shared` from `CXXFLAGS` for this to work.

## Interesting Files
- Take a look at `features.py` to see how I implemented some heuristic analysis. The code is quite commented and should be fairly easy to understand, but the python version is not very effecient.  `features.cpp` implements basically all of these features with a new shogi API except for the last couple of the board shape.
- Of course, `evolve.py` has quite a bit going on as well, but here too it is commented well so it should be relatively easy to follow. There are some funky design choices about global definitons, but this is mostly due to some things I could not controll about the packages I was using / multiprocessing. But see comments for further details.
- The other one-off scripts I used to *borrow* and clean up my database of games is in `/screipts`

## References
- Python Libraries:
  - [DEAP](https://deap.readthedocs.io/en/master/)
  - [Pyhthon Shogi](https://github.com/gunyarakun/python-shogi)
  - [Cpp Shogi](https://github.com/KILABRZ/cppEasyShogi)
  - [Open MP](https://www.openmp.org/)
  - [Open GA](https://github.com/Arash-codedev/openGA)

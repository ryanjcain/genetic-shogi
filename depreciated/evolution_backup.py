import shogi
import numpy
import time
import random
import itertools
import multiprocessing as mp

from deap import base
from deap import creator
from deap import tools
from deap import algorithms

# Import my configuration of algorithm
import config as cfg

# Functions to help with Gray encodings
def bin2gray(bits):
    return bits[:1] + [i ^ ishift for i, ishift in zip(bits[:-1], bits[1:])]

def gray2bin(bits):
    b = [bits[0]]
    for nextb in bits[1:]: b.append(b[-1] ^ nextb)
    return b

def gray_bits_to_weights(individual, width):
    '''
    Take an individual list of N bits and break it into N / width gray encoded segments,
    then decodes them to integers. Caches mean a O(1) lookup!
    '''
    
    return [bit2int_cache[gray2bit_cache[tuple(individual[i:i+width])]] 
            for i in range(0, len(individual), width)]
    
# Heuristic function must have global scope for multiprocessing
def H(action, weights):
    move, resulting_pos = action[0], action[1]
    value = sum([cfg.SHOGI_FEATURES[i](resulting_pos) * weights[i] 
                      for i in range(cfg.NUM_FEATURES)])
    return move, value

# Evaluation (fitness) function
def grandMasterEval(individual):
    '''
    This is the Evaluation function of an individual as described in the paper 
    'Genetic Algorithms for Evolving Computer Chess Programs' by Eli David, 
    H. Herik, M. Koppel, and N. Netanyahu. 
    '''
    
    # Decode the organism's bit string to a set of integer weights
    weights = gray_bits_to_weights(individual, cfg.BIT_WIDTH_SMALL)
    
    pool = mp.Pool(mp.cpu_count())

    # Iterate through sample of grandmaster games to see peformance of heuristic
    correct = 0
    game_no = 0
    eval_tm = 0
    positions = 0

    for game in cfg.TRAIN:
        # Initialize the board to the reference game
        pos, grandmaster_move = game[0], game[1]
        eval_start = time.time()

        # Uncomment for paraallelization
        results = pool.starmap_async(H, [(action, weights) for action in cfg.legal_moves_cache[pos]]).get()
        best_action = max(results, key=lambda action: action[1]) 
        best_move, value = best_action[0], best_action[1]

        # Parallel version of map! -- Do not think this works with duel paralelization (of organisms and feature calc.)
        # results = toolbox.map(H, [(action, weights) for action in legal_moves_cache[pos]])
        # best_action = max(results, key=lambda action: action[1]) 
        # best_move, value = best_action[0], best_action[1]

        # UNCOMMENT FOR NO GAME LEVEL PARALLELIZATION
        # Perform 1-ply search for best pos using decoded heuristic function
        # best_move, best_val = "", 0
        # for action in cfg.legal_moves_cache[pos]:
        #     # Evalueate the heuristic to see the value of the action
        #     move, value = H(action, weights)
            
        #     # Keep track of the best seen so far
        #     if value > best_val:
        #         best_move = move
        #         best_val = value

        # # Update if the Heuristic guessed correctly
        if best_move == grandmaster_move:
            correct += 1

        game_no += 1
        positions += len(cfg.legal_moves_cache[pos])
        # eval_tm += time.time() - eval_start
        print("Game eval took: {}s".format(time.time() - eval_start))

    print("Evaluated {} games, {} positions, avg. time / game: {}s".format(len(cfg.TRAIN), positions, eval_tm / len(cfg.TRAIN)))


    # Uncomment for Parallelization
    pool.close()

    # Overall fitness is the square of total number of correct moves
    fitness = correct * correct
    return fitness, 


# Functions modified from deap libary
def varAnd(population, toolbox, cxpb, mutpb):
    """
    Coppied from the deap library so I could modify for logging
    purposes.
    """
    offspring = [toolbox.clone(ind) for ind in population]

    # Apply crossover and mutation on the offspring
    for i in range(1, len(offspring), 2):
        if random.random() < cxpb:
            offspring[i - 1], offspring[i] = toolbox.mate(offspring[i - 1],
                                                          offspring[i])
            del offspring[i - 1].fitness.values, offspring[i].fitness.values

    for i in range(len(offspring)):
        if random.random() < mutpb:
            offspring[i], = toolbox.mutate(offspring[i])
            del offspring[i].fitness.values

    return offspring


def eaSimple(population, toolbox, cxpb, mutpb, ngen, stats=None,
             halloffame=None, verbose=__debug__):
    """ 
    Modified version of the simple evolutionary algorithm provided in deap
    to facilitate slightly better logging.
    """

    last_gen = time.time()

    logbook = tools.Logbook()
    logbook.header = ['gen', 'nevals', 'time'] + (stats.fields if stats else [])

    # Evaluate the individuals with an invalid fitness
    invalid_ind = [ind for ind in population if not ind.fitness.valid]
    fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = fit

    if halloffame is not None:
        halloffame.update(population)

    record = stats.compile(population) if stats else {}

    # Added for logging purposes
    c = time.time() - last_gen
    hrs = int(c // 3600 % 24)
    mins = int(c // 60 % 60)
    secs = int(c % 60)
    last_gen = time.time()


    logbook.record(gen=0, nevals=len(invalid_ind), time="[{}:{}:{}]".format(hrs, mins, secs), **record)
    if verbose:
        cfg.log(logbook.stream)

    # Begin the generational process
    for gen in range(1, ngen + 1):
        # Implement elitism, save the best member from each generation
        elitist = toolbox.clone(max(population, key=lambda ind: ind.fitness.values))

        # Select the next generation individuals
        offspring = toolbox.select(population, len(population) - 1)

        # Vary the pool of individuals
        offspring = varAnd(offspring, toolbox, cxpb, mutpb)

        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        # Update the hall of fame with the generated individuals, add back elitist
        offspring.append(elitist)
        if halloffame is not None:
            halloffame.update(offspring)

        # Replace the current population by the offspring + elitist
        population[:] = offspring

        # Append the current generation statistics to the logbook
        record = stats.compile(population) if stats else {}\
        
        # Added for logging purposes
        c = time.time() - last_gen
        hrs = int(c // 3600 % 24)
        mins = int(c // 60 % 60)
        secs = int(c % 60)
        last_gen = time.time()

        logbook.record(gen=gen, nevals=len(invalid_ind), time="[{}:{}:{}]".format(hrs, mins, secs), **record)
        if verbose:
            cfg.log(logbook.stream)
        
        # Global bookeeping for progressbar
        cfg.PROGRESS_BAR.update(gen)

    return population, logbook



def evolve(toolbox):
    '''
    Driver function for the genetic algrotihm
    '''

    pop = toolbox.population(n=cfg.POP_SIZE)
    hof = tools.HallOfFame(1)
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("avg", numpy.mean)
    stats.register("std", numpy.std)
    stats.register("min", numpy.min)
    stats.register("max", numpy.max)


    cfg.PROGRESS_BAR.start()
    eaSimple(pop, toolbox, cxpb=cfg.CXPB, mutpb=cfg.MUTPB, ngen=cfg.N_GEN, 
             stats=stats, halloffame=hof, verbose=True)
    cfg.PROGRESS_BAR.finish()

    return pop, stats, hof

creator.create("FitnessMax", base.Fitness, weights=(1.0,))       # Single objective max
creator.create("Individual", list, fitness=creator.FitnessMax)   # Define individual class


toolbox = base.Toolbox()

# Define how we will initialize an individual and the population
toolbox.register("attr_bool", random.randint, 0, 1)             # Attribure generator
toolbox.register("individual", tools.initRepeat,                # Individual initializer
                creator.Individual, toolbox.attr_bool,      
                cfg.CHROMOSOME_LEN)
toolbox.register("population", tools.initRepeat, list, toolbox.individual)                              

# Register the built-ins and our own evaluation function
toolbox.register("evaluate", grandMasterEval, )
toolbox.register("mate", tools.cxUniform, indpb=0.4)
toolbox.register("mutate", tools.mutFlipBit, indpb=0.05)
toolbox.register("select", tools.selRoulette)

gray2bit_cache = {
    tuple(bin2gray(list(x))): tuple(x)
    for x in itertools.product((0,1), repeat=cfg.BIT_WIDTH_SMALL)
}

digits = ['0', '1']
bit2int_cache = {
    tuple(x): int("".join([digits[y] for y in x]), 2)
    for x in itertools.product((0,1), repeat=cfg.BIT_WIDTH_SMALL)
}



# for game in cfg.TRAIN:
#     pos, move = game[0], game[1]
#     board = shogi.Board(pos)

#     actions = []
#     for move in board.legal_moves:
#         # Make the move and record the str representation of the new position=
#         board.push(move)
#         resulting_pos_str = board.sfen()

#         # Store usi move and a new board object of the resulting position
#         action = (move.usi(), shogi.Board(resulting_pos_str))
#         actions.append(action)

#         # Reset the board state
#         board.pop()

#     # Add all of the move, resulitng board pairs to the dictionary
#     legal_moves_cache[pos] = actions

def log_params(console=False):
    cfg.log("---------- Misc Settings ---------", console=console)
    cfg.log("Utilizing {} CPUs".format(mp.cpu_count()), console=console)
    cfg.log("Chromosome length: {}".format(cfg.CHROMOSOME_LEN), console=console)
    cfg.log("Bit width: {}".format(cfg.BIT_WIDTH_SMALL), console=console)
    cfg.log("Shogi Features: {}".format(cfg.NUM_FEATURES), console=console)

    cfg.log("\n---------- GA Parameters ---------", console=console)
    cfg.log("Training data (N) = {}".format(len(cfg.TRAIN)), console=console)
    cfg.log("Population size = {}".format(cfg.POP_SIZE), console=console)
    cfg.log("Crossover Probability = {}".format(cfg.CXPB), console=console)
    cfg.log("Mutation Probability = {}".format(cfg.MUTPB), console=console)
    cfg.log("Generations = {}".format(cfg.N_GEN), console=console)

    if not console:
        cfg.log("\n---------- GA Results -----------")

def main():
    '''
    Driver for the evolutionary algorithm, handles parallelization, GA execution
    as well as logging.
    '''

    # Small software cache for converting betwen binary and gray encodings
    # global gray2bit_cache
    # gray2bit_cache = {
    #     tuple(bin2gray(list(x))): tuple(x)
    #     for x in itertools.product((0,1), repeat=cfg.BIT_WIDTH)
    # }

    # # Software cache for converting 6-bit binary subsequence to integer (used in evaluation)
    # digits = ['0', '1']
    # global bit2int_cache
    # bit2int_cache = {
    #     tuple(x): int("".join([digits[y] for y in x]), 2)
    #     for x in itertools.product((0,1), repeat=cfg.BIT_WIDTH)
    # }



    # --------------------------- Genetic Algorithm Framework --------------------------- 
    
    # Use deap to speficy we want a simple max of a single objective, with idividuals 
    # represented as a Gray encoded list of bits. 
    

   

    # creator.create("FitnessMax", base.Fitness, weights=(1.0,))       # Single objective max
    # creator.create("Individual", list, fitness=creator.FitnessMax)   # Define individual class

    # global toolbox
    # toolbox = base.Toolbox()

    # # Define how we will initialize an individual and the population
    # toolbox.register("attr_bool", random.randint, 0, 1)             # Attribure generator
    # toolbox.register("individual", tools.initRepeat,                # Individual initializer
    #                 creator.Individual, toolbox.attr_bool,      
    #                 cfg.CHROMOSOME_LEN)
    # toolbox.register("population", tools.initRepeat, list, toolbox.individual)                              

    # # Register the built-ins and our own evaluation function
    # toolbox.register("evaluate", grandMasterEval, )
    # toolbox.register("mate", tools.cxUniform, indpb=0.4)
    # toolbox.register("mutate", tools.mutFlipBit, indpb=0.05)
    # toolbox.register("select", tools.selRoulette)


    # # cores = mp.cpu_count()
    # MANAGER = mp.Manager()

    # # # Create a cache to store all possible legal moves for each of the N game positions
    # print("--------------- Caching Legal Moves ---------------")
    # global legal_moves_cache
    # legal_moves_cache = MANAGER.dict()

    # for game in cfg.TRAIN:
    #     pos, move = game[0], game[1]
    #     board = shogi.Board(pos)

    #     actions = []
    #     for move in board.legal_moves:
    #         # Make the move and record the str representation of the new position=
    #         board.push(move)
    #         resulting_pos_str = board.sfen()

    #         # Store usi move and a new board object of the resulting position
    #         action = (move.usi(), shogi.Board(resulting_pos_str))
    #         actions.append(action)

    #         # Reset the board state
    #         board.pop()

    #     # Add all of the move, resulitng board pairs to the dictionary
    #     legal_moves_cache[pos] = actions


    # NOTE : maybe do not store the MOVE object but instead just store the string?
    start = time.time()

    # Reset log file
    open("log.txt", "w").close()

    log_params(console=True)

    # Perform the evolution
    print("--------------- Beginning Evolution ---------------")
    # pool = mp.Pool(mp.cpu_count())
    # toolbox.register("map", pool.map)
    pop, stats, hof = evolve(toolbox)
    # pool.close()
    # Record how long it took 
    end = time.time() - start
    hrs = int(end // 3600 % 24)
    mins = int(end // 60 % 60)
    secs = int(end % 60)
    cfg.log("\nFinished evolution, took [{}:{}:{}]s".format(hrs, mins, secs))
    cfg.log("\n--------------- Final popluation ---------------")
    if pop:
        for ind in pop:
            for weight in gray_bits_to_weights(ind, cfg.BIT_WIDTH_SMALL):
                cfg.log("{},".format(weight), end=" ")
            cfg.log("\n")
        # cfg.log("\n")

    cfg.log("--------------- Best Individual ---------------")
    for weight in gray_bits_to_weights(hof[0], cfg.BIT_WIDTH_SMALL):
        cfg.log("{},".format(weight), end=" ")


if __name__ == "__main__":
    main()

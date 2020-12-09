import itertools
import progressbar
import random
import time

import numpy
from deap import base, creator, tools

# Import my libraries
from eAlgos import eaSimple
from logger import gaLogger
from gray import GrayEncoder
from config import params as cfg

# Try importing C++ library
try:
    import GeneticShogi as gs
except ModuleNotFoundError as e:
    print("Could not find GeneticShogi Package.")
    print("Make sure:")
    print("  1. Python3 virtual environment is activated")
    print("  2. Dependencies are installed: pip3 install -r requirements.txt.")
    print("  3. Genetic Shogi module is compiled and built in cpp/ directory.")
    exit(-1)


# ---------------------- Global Variables ---------------------- 

# Global organism evaluator from GeneticShogi python package built from C++
EVALUATOR = gs.OrganismEvaluator()
EVALUATOR.set_mode(cfg['eval_mode'])
EVALUATOR.set_num_eval(cfg['n_train'])

# Global encoder / decoder to store bit caches used in gray bit to int conversion
ENCODER = GrayEncoder(cfg['bit_width_small'], 
                      cfg['bit_width_wide'], 
                      split=EVALUATOR.get_num_major_features() *
                      cfg['bit_width_wide'])

# Add variables computed based no evaluator to config
major_features = EVALUATOR.get_num_major_features()
other_features = EVALUATOR.get_num_features() - major_features
chromosome_len = cfg['bit_width_small'] * other_features + major_features * cfg['bit_width_wide']
cfg['num_major_features'] = major_features
cfg['num_other_features'] = other_features
cfg['chromosome_len'] = chromosome_len

# -------------------------------------------------------------- 


def init_progressbar(n_gen):
    '''
    Simple progress bar initializer
    '''
    widgets = [
        ' [',
        progressbar.Timer(), '] ',
        progressbar.Bar(), ' (',
        progressbar.AdaptiveETA(), ') ',
        progressbar.Percentage()
    ]

    return progressbar.ProgressBar(maxval=n_gen, widgets=widgets)

def grandMasterEval(individual):
    '''
    This is the main function called in the fitness evaluation process for each
    of the individual chromosomes. It invokes either a C++ implementation of
    the evaluation function as described by the paper 'Genetic Algorithms for
    Evolving Computer Chess Programs' by Eli David, H. Herik, M. Koppel, and N. Netanyahu.
    '''

    weights = ENCODER.gray_bits_to_weights(individual)

    # Print raw weights to command line
    if cfg['verbose']:
        print(weights)

    # Record evaluation time of the individual for logging reports
    start = time.time()

    # Call the evaluator in the GeneticShogi C++ library
    fitness = EVALUATOR.evaluate_organism(weights)

    # Must be a tuple as specified in DEAP documentation
    return fitness,


def init_ga_toolbox():
    '''
    Initialize the DEAP genetic algorithm
    '''
    creator.create("FitnessMax", base.Fitness, weights=(1.0, ))  # Single objective max
    creator.create("Individual", list, fitness=creator.FitnessMax)  # Define individual class

    # Base toolbox from the DEAP module
    toolbox = base.Toolbox()

    # Define how we will initialize an individual and the population
    toolbox.register("attr_bool", random.randint, 0, 1)  # Attribure generator
    toolbox.register( "individual",
        tools.initRepeat,  # Individual initializer
        creator.Individual,
        toolbox.attr_bool,
        cfg['chromosome_len'])
    toolbox.register("population", tools.initRepeat, list, toolbox.individual)

    # Register the built-ins and our own evaluation function
    toolbox.register(
        "evaluate",
        grandMasterEval,
    )
    toolbox.register("mate", tools.cxUniform, indpb=0.4)
    toolbox.register("mutate", tools.mutFlipBit, indpb=0.05)
    toolbox.register("select", tools.selRoulette)

    return toolbox


def evolve(toolbox, log, prog_bar):
    '''
    Driver function for the Entire genetic algorithm. Sets up statistics
    for logging purpouses and starts the genertional process by calling
    eaSimple().
    '''

    pop = toolbox.population(n=cfg['pop_size'])
    hof = tools.HallOfFame(1)
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("avg", numpy.mean)
    stats.register("std", numpy.std)
    stats.register("min", numpy.min)
    stats.register("max", numpy.max)

    pawn_val = '1100100'.zfill(cfg['bit_width_wide'])
    pawn_val = [int(i) for i in pawn_val]
    print("Pawn value: ", pawn_val)
    print("Pawn value gray: ", ENCODER.bin2gray(pawn_val))
    pawn_gene =  {'start': 0, 
                  'stop': cfg['bit_width_wide'], 
                  'value': ENCODER.bin2gray(pawn_val)
                  }

    prog_bar.start()

    eaSimple(pop, toolbox, cxpb=cfg['cxpb'], mutpb=cfg['mutpb'], ngen=cfg['n_gen'],
             stats=stats, halloffame=hof, logger=log, verbose=True, bar=prog_bar,
             const_gene=pawn_gene)

    prog_bar.finish()

    return pop, stats, hof


def main():
    '''
    Driver for the evolutionary algorithm, handles parallelization, GA execution
    as well as logging.
    '''

    # Initialize the functions used by the genetic algorithm
    toolbox = init_ga_toolbox()

    # Initalize the ascii progress bar
    bar = init_progressbar(cfg['n_gen'])


    # Labels of the features being used
    labels = EVALUATOR.get_feature_labels()
    cfg['labels'] = labels

    # Instantiate logger for the genetic algorithm and pring out config parameters
    logger = gaLogger(cfg['log_file'])
    logger.log_organism_ga_params(cfg, console=True)
    logger.log_organism_ga_params(cfg, console=False)

    # Run the evolutionary algorithm, close multiprocessing pool when completed.
    print("--------------- Beginning Evolution ---------------")
    start = time.time()
    pop, stats, hof = evolve(toolbox, logger, bar)

    # Record how long EA took
    end = time.time() - start
    hrs = int(end // 3600 % 24)
    mins = int(end // 60 % 60)
    secs = int(end % 60)
    logger.log("\nFinished evolution, look at {} for results.".format(
        cfg['log_file']),
            console=True)

    logger.log("\nEvolution took [{}:{}:{}]s".format(hrs, mins, secs),
            console=False)

    # Record the best individual from all N generations
    logger.log("\n--------------- Best Individual ---------------")
    best_weights = ENCODER.gray_bits_to_weights(hof[0])
    for i, label in enumerate(labels):
        logger.log("{}: {}".format(label, best_weights[i]), end="\n")

    # Record the Final Population
    logger.log("\n\n--------------- Final popluation ---------------")
    if pop:
        for individual in pop:
            weights = ENCODER.gray_bits_to_weights(individual)
            for weight in weights:
                logger.log("{},".format(weight), end=" ")
            logger.log("\n")


if __name__ == "__main__":
    main()

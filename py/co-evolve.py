import random
import numpy
from deap import base, creator, tools

# Import my libraries
from eAlgos import eaSimple
from logger import gaLogger
from gray import GrayEncoder
from coEvolveConfig import params as cfg
from config import params as orgCfg

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

SENTE = 0
GOTE = 1

INITAL_POP = []
with open(orgCfg['organism_file'], "r") as pop_file:
    top_organisms = pop_file.readlines()
    for org in top_organisms:
        org_array = []
        for char in org:
            if char == '0' or char == '1':
                org_array.append(int(char))
    INITAL_POP.append(org_array)

EVALUATOR = gs.OrganismEvaluator()
ENCODER = GrayEncoder(orgCfg['bit_width_small'], 
                      orgCfg['bit_width_wide'], 
                      split=EVALUATOR.get_num_major_features() *
                      orgCfg['bit_width_wide'])
ORGANISMS = [ENCODER.gray_bits_to_weights(ind) for ind in INITAL_POP]

def gameSimulationEval(individual):
    '''
    Evaluate an individual by seeing how many times it beats another.
    '''

    individual = ENCODER.gray_bits_to_weights(individual)

    opponents = [random.choice(ORGANISMS) for i in range(cfg['n_simulate'])]
    
    if cfg['verbose']:
        print(individual)
    
    game = None
    player = random.randint(0, 1)

    wins = 0
    game = None
    for opponent in opponents:
        if player == SENTE:
            game = gs.OrganismGame(individual, opponent, cfg['max_turns'], cfg['max_search_depth'])
        else:
            game = gs.OrganismGame(opponent, individual, cfg['max_turns'], cfg['max_search_depth'])

        outcome = game.simulate()
        if game == player:
            wins += 1

    return wins,


def init_ga_toolbox():
    '''
    Initialize the DEAP genetic algorithm
    '''
    creator.create("FitnessMax", base.Fitness, weights=(1.0, ))  # Single objective max
    creator.create("Individual", list, fitness=creator.FitnessMax)  # Define individual class
    
    def initIndividual(icls, organism):
        org = ""
        for char in organism:
            if char == '0' or char == '1':
                # org += (int(char))
                org += char
        return icls([int(char) for char in org])
        # return icls(org)


    def initPopulation(pcls, ind_init, filename):
        INITAL_POP = []
        with open(filename, "r") as pop_file:
            top_organisms = pop_file.readlines()

        return pcls(ind_init(c) for c in top_organisms)

    # Base toolbox from the DEAP module
    toolbox = base.Toolbox()

    toolbox.register("individual_guess", initIndividual, creator.Individual)
    toolbox.register("population", initPopulation, list, toolbox.individual_guess, orgCfg['organism_file'])

    # Define how we will initialize an individual and the population
    toolbox.register("attr_bool", random.randint, 0, 1)  # Attribure generator
    toolbox.register( "individual",
        tools.initRepeat,  # Individual initializer
        creator.Individual,
        toolbox.attr_bool,
        len(INITAL_POP[0]))

    # Register the built-ins and our own evaluation function
    toolbox.register("evaluate", gameSimulationEval)
    toolbox.register("mate", tools.cxUniform, indpb=0.4)
    toolbox.register("mutate", tools.mutFlipBit, indpb=0.05)
    toolbox.register("select", tools.selRoulette)

    return toolbox


def evolve(toolbox, log=None, prog_bar=None):
    '''
    Driver function for the Entire genetic algorithm. Sets up statistics
    for logging purpouses and starts the genertional process by calling
    eaSimple().
    '''

    pop = toolbox.population()
    hof = tools.HallOfFame(1)
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("avg", numpy.mean)
    stats.register("std", numpy.std)
    stats.register("min", numpy.min)
    stats.register("max", numpy.max)

    eaSimple(pop, toolbox, cxpb=cfg['cxpb'], mutpb=cfg['mutpb'], ngen=cfg['n_gen'],
             stats=stats, halloffame=hof, logger=log, verbose=cfg['verbose'])

    return pop, stats, hof


def main():
    toolbox = init_ga_toolbox()
    pop, stats, hof = evolve(toolbox)
    for weight in ENCODER.gray_bits_to_weights(hof[0]):
        print(weight)

if __name__ == "__main__":
    main()





    
    



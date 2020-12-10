import random
import time
from deap import tools

'''
I do not take credit for this. These algorithms are part of the DEAP library,
I have simply modified to support my own logging class.
'''

def varAnd(population, toolbox, cxpb, mutpb):
    """
    Modified code from Deap library for mostly logging purposes.
    Allows for BOTH crossover and mutation to occur on the same organism.
    This is as opposed to some evoutionary models that make these two
    opperations mutually exclusive.
    """
    offspring = [toolbox.clone(ind) for ind in population]

    # Apply crossover and mutation on the offspring
    for i in range(1, len(offspring), 2):
        if random.random() < cxpb:
            offspring[i - 1], offspring[i] = toolbox.mate(
                offspring[i - 1], offspring[i])
            del offspring[i - 1].fitness.values, offspring[i].fitness.values

    for i in range(len(offspring)):
        if random.random() < mutpb:
            offspring[i], = toolbox.mutate(offspring[i])
            del offspring[i].fitness.values

    return offspring

def set_gene_value(population, gene):
    '''
    Set specified gene to value for every individual ine population. A gene is specified by its
    start and end bit.
    '''
    for ind in population:
        ind[gene['start']:gene['stop']] = gene['value']

def eaSimple(population,
             toolbox,
             cxpb,
             mutpb,
             ngen,
             stats=None,
             halloffame=None,
             logger=None,
             verbose=__debug__,
             bar=None,
             const_gene=None):
    """
    Modified version of the simple evolutionary algorithm provided in DEAP
    to facilitate my own logger and progressbar.
    """
    if const_gene:
        set_gene_value(population, const_gene)

    last_gen = time.time()

    logbook = tools.Logbook()
    logbook.header = ['gen', 'nevals', 'time'
                      ] + (stats.fields if stats else [])

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

    logbook.record(gen=0,
                   nevals=len(invalid_ind),
                   time="[{}:{}:{}]".format(hrs, mins, secs),
                   **record)
    if verbose and logger:
        logger.log(logbook.stream)

    # Begin the main genertional process
    for gen in range(1, ngen + 1):
        # print("---------- Generation {} ---------- ".format(gen))
        # Implement elitism, save the best member from each generation
        elitist = toolbox.clone(
            max(population, key=lambda ind: ind.fitness.values))

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

        if const_gene:
            set_gene_value(population, const_gene)

        # Append the current generation statistics to the logbook
        record = stats.compile(population) if stats else {}\

        # Added for logging purposes
        c = time.time() - last_gen
        hrs = int(c // 3600 % 24)
        mins = int(c // 60 % 60)
        secs = int(c % 60)
        last_gen = time.time()

        logbook.record(gen=gen,
                       nevals=len(invalid_ind),
                       time="[{}:{}:{}]".format(hrs, mins, secs),
                       **record)

        if verbose and logger:
            logger.log(logbook.stream)

        # Global bookeeping for progressbar
        if bar:
            bar.update(gen)

    return population, logbook


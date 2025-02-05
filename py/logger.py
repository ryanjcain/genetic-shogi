from config import params

# Function to facilitate logging

class gaLogger:
    '''
    Control logging throughout the course of the evolutionary algorithm
    '''
    def __init__(self, log_file):
        self.file = log_file

        # Reset log file
        open(self.file, "w").close()


    def log(self, message, console=False, end="\n"):
        if not console:
            with open(self.file, "a") as f:
                f.write(message)
                f.write(end)
        else:
            print(message)


    def log_organism_ga_params(self, params, console=False):
        self.log("---------- Misc Settings ---------", console=console)
        self.log("Evaluation Language: {}".format(params['eval_lang']), console=console)
        self.log("Evaluation mode: {}".format(params['eval_mode']), console=console)
        self.log("Number of major features: {}".format(params['num_major_features']), console=console)
        self.log("Number of other (minor) features: {}".format(params['num_other_features']), console=console)
        self.log("Total Features: {}".format(params['num_major_features'] + params['num_other_features']), console=console)
        self.log("Train positions = {}".format(params['n_train']), console=console)
        self.log("Test  positions = {}".format(params['n_test']), console=console)
        self.log("Log File = {}".format(params['log_file']), console=console)

        self.log("\n---------- GA Parameters ---------", console=console)
        self.log("Population size = {}".format(params['pop_size']), console=console)
        self.log("Crossover Probability = {}".format(params['cxpb']), console=console)
        self.log("Mutation Probability = {}".format(params['mutpb']), console=console)
        self.log("Generations = {}".format(params['n_gen']), console=console)
        print()
        self.log("Bit width small (Minor): {}".format(params['bit_width_small']), console=console)
        self.log("Bit width wide (Major): {}".format(params['bit_width_wide']), console=console)
        self.log("Chromosome Length: {}".format(params['chromosome_len']), console=console)
        print()
        self.log("\n---------- Eval Features ---------", console=console)
        for i, feature in enumerate(params['labels']):
            feature_type = ""
            if i < params['num_major_features']:
                feature_type = "Major"
            else:
                feature_type = "Minor"
            self.log("{}: {}".format(feature, feature_type), console=console)

        if not console:
            self.log("\n---------- GA Results -----------")

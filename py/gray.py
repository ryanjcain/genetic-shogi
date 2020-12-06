import itertools

class GrayEncoder:
    '''
    Simple class to effieciently handle the encoding and decoding of binary strings,
    gray binary strings, and base 10 numbers. Supports using multiple bitwidths at the 
    specified cuttoff. So bits before the cuttoff will be translated with the wide bit
    width and those following the cuttof will be interpreted with the smaller bit width.
    If no cuttoff is provided, the whole sequence will be interpreted by the smaller 
    bit width.
    '''
    def __init__(self, bit_width, bit_width_wide=0, split=0):
        self.width = bit_width
        self.width_wide = bit_width_wide

        # Cuttoff if multiple bit widths provided
        self.cutoff = split
        
        # Caches for encoding and decoding gray chromosomes
        self.gray2bit_cache = {
            tuple(GrayEncoder.bin2gray(list(x))): tuple(x)
            for x in itertools.product((0, 1), repeat=self.width)
        }
        
        digits = ['0', '1']
        self.bit2int_cache = {
            tuple(x): int("".join([digits[y] for y in x]), 2)
            for x in itertools.product((0, 1), repeat=self.width)
        }

        # Enlarge with the wider bits if necessary
        if self.width_wide:
            self.gray2bit_cache_large = {
                tuple(GrayEncoder.bin2gray(list(x))): tuple(x)
                for x in itertools.product((0, 1), repeat=self.width_wide)
            }
            self.gray2bit_cache.update(self.gray2bit_cache_large)
        
            self.bit2int_cache_large = {
                tuple(x): int("".join([digits[y] for y in x]), 2)
                for x in itertools.product((0, 1), repeat=self.width_wide)
            }
            self.bit2int_cache.update(self.bit2int_cache_large)

    @staticmethod
    def bin2gray(bits):
        return bits[:1] + [i ^ ishift for i, ishift in zip(bits[:-1], bits[1:])]

    @staticmethod
    def gray2bin(bits):
        b = [bits[0]]
        for nextb in bits[1:]:
            b.append(b[-1] ^ nextb)
        return b


    def gray_bits_to_weights(self, seq):
        '''
        Decode the the individual (gray encoded bit string) to a set of integer weights
        '''
        piece_weights = self._gray_bits_to_ints(seq[0:self.cutoff], self.width_wide)
        other_weights = self._gray_bits_to_ints(seq[self.cutoff:], self.width)

        weights = piece_weights + other_weights

        return weights
    
    def _gray_bits_to_ints(self, individual, width):
        '''
        Take an individual list of N bits and break it into N / width gray encoded segments,
        then decodes them to integers. Caches mean a O(1) lookup!
        '''
        return [
            self.bit2int_cache[self.gray2bit_cache[tuple(individual[i:i + width])]]
            for i in range(0, len(individual), width)
        ]


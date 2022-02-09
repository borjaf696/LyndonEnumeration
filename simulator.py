# Borja :)
import numpy as np
import os, sys, string
import itertools

def __simulate(sigma, length):
    np.random.seed(6543210)
    list_chars = [t for t in string.ascii_uppercase]
    sigma = sigma if sigma < len(list_chars) else len(list_chars)
    alphabet = np.random.choice(list_chars, size = sigma+1, replace = False)
    name_f = 'simulations/sim_'+str(sigma)+'_'+str(length)+'.txt'
    with open(name_f,'w+') as f: 
        for i in [alphabet[t] for t in np.random.randint(0,high = len(alphabet) - 1, size = length)]:
            f.write(i)

if __name__ == '__main__':
    print('python simulator.py sigma len')
    sigma, length = int(sys.argv[1]), int(sys.argv[2])
    min_sigma, min_length = 2, 10
    list_sigma,list_length = list(range(min_sigma,sigma)), list(range(min_length, length,min_length))
    for (sig, l) in itertools.product(list_sigma, list_length):
        __simulate(sig,l)
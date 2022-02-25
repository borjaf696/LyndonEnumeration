# Borja :)
import numpy as np
import os, sys, string
import itertools

LOW_RANGE_SIGMA = 4
LOW_RANGE_LENGTH = 30
def __simulate(sigma, length, num_sim, seed, dir = 'simulations/', local_sims = 1):
    np.random.seed(seed)
    sim_dir = dir+str(num_sim)+'/'
    if not os.path.exists(sim_dir):
        os.mkdir(sim_dir)
    if not os.path.exists('output'):
        os.mkdir('output')
    list_chars = [t for t in string.ascii_uppercase]
    sigma = sigma if sigma < len(list_chars) else len(list_chars)
    alphabet = np.random.choice(list_chars, size = sigma+1, replace = False)
    def __simule(name_f):
        with open(name_f,'w+') as f: 
            for i in [alphabet[t] for t in np.random.randint(0,high = len(alphabet) - 1, size = length)]:
                f.write(i)
    name_f = sim_dir+'sim_'+str(sigma)+'_'+str(length)
    [__simule(name_f+'_'+str(i)+'.txt') for i in range(local_sims)]
    
if __name__ == '__main__':
    print('python simulator.py sigma len')
    nsims, local_sims = 30, 100
    sigma, length, seed = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
    min_sigma, min_length = 2, 10
    for i in range(nsims):
        list_sigma,list_length = list(range(min_sigma,sigma)), list(range(min_length, length,int(min_length/2)))
        for (sig, l) in itertools.product(list_sigma, list_length):
            if sig <= LOW_RANGE_SIGMA and l <= LOW_RANGE_LENGTH:
                __simulate(sig,l,i, seed, local_sims = local_sims)
            else:
                __simulate(sig,l,i, seed)
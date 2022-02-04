# Borja :)
import numpy as np
import os, sys, string

def __simulate(sigma, length):
    np.random.seed(6543210)
    list_chars = [t for t in string.ascii_uppercase]
    sigma = sigma if sigma < len(list_chars) else len(list_chars)
    alphabet = np.random.choice(list_chars, size = sigma, replace = False)
    with open('example/simulation.txt','w+') as f: 
        for i in [alphabet[t] for t in np.random.randint(0,high = len(alphabet) - 1, size = length)]:
            f.write(i)

if __name__ == '__main__':
    print('python simulator.py sigma len')
    sigma, length = int(sys.argv[1]), int(sys.argv[2])
    __simulate(sigma, length)
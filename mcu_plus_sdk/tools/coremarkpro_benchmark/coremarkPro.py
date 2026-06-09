
# # Calculate the overall CoreMark Pro score
import argparse
import sys
import math
from functools import reduce
def compute_overall_score(C):

    n = len(C)  # Number of elements (workloads)
    R = [40.3438, 2855, 38.5624, 0.87959, 1.45853, 4.81116, 99.6587, 48.5201, 21.3618]  # Example R_i values
    S = [1,10000,1,1,1,1,1,1,1]     # Example S_i values
    # Ensure all input lists have the same length
    if not (len(C) == len(R) == len(S)):
        raise ValueError("C, R, and S must have the same number of elements.")

    # Compute the product inside the formula
    product = 1
    for i in range(n):
        product *= (C[i] / R[i]) * S[i]

    # Compute the nth root of the product and scale by 1000
    overall_score = 1000 * (product ** (1 / n))
    return overall_score


def sync_main(argv):
    # SCRIPT START

    # Argument parsing
    arg_parser = argparse.ArgumentParser(description="Compute overall Coremark Pro benchmark")
    arg_parser.add_argument('--cjpeg',  dest='cjpeg',   type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--core',   dest='core',    type=float,   help='Core worload score')
    arg_parser.add_argument('--lin',    dest='lin',     type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--loops',  dest='loops',   type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--nnet',   dest='nnet',    type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--parser', dest='parser',  type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--radix',  dest='radix',   type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--sha',    dest='sha',     type=float,   help='Cjpeg worload score')
    arg_parser.add_argument('--zip',    dest='zip',     type=float,   help='Cjpeg worload score')

    args = vars(arg_parser.parse_args())

    C = list(args.values())
    result = compute_overall_score(C)
    print("Overall Score:", result)

if __name__ == "__main__":
    sync_main(sys.argv[1:])
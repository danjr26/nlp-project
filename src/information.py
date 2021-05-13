import math
import collections
import time

def mutual_info(s1, s2):
	assert(len(s1) == len(s2))
	counts1 = collections.Counter(s1)
	counts2 = collections.Counter(s2)
	joint_counts = collections.Counter(zip(s1, s2))
	
	total = 0
	for c1, c2 in zip(s1, s2):
		prob1 = counts1[c1] / len(s1)
		prob2 = counts2[c2] / len(s2)
		joint_prob = joint_counts[(c1, c2)] / len(s1)
		total += joint_prob * math.log2(joint_prob / (prob1 * prob2))
	return total
	
if __name__ == "__main__":
	info = mutual_info('111111100', '111111100')
	print(info)
import math
from sentence import Sentence
from information import mutual_info

class Node:
	def __init__(self):
		pass

	def forward(self):
		pass

class SubNode:
	def __init__(self):
		pass

class InputNode(Node):
	def __init__(self, vocab):
		self.vocab_size = len(vocab)

class InputSubNode(Node):
	def __init__(self, c, vocab_size):
		super.__init__()
		self.c = c
		self.x = 1
		self.n = vocab_size

	def observe(self, c):
		self.x += int(c == self.c)
		self.n += 1
	

class Network:
	def __init__(self, input_size:int, vocab:set):
		self.input_size = input_size
		self.vocab_size = len(vocab)
		self.vocab = vocab
		base = [BaseNode() for i in range(self.input_size)]

	def train(self, sentence:Sentence):
		pass

	def logprob_sentence(self, sentence:Sentence):
		for i in range(-self.input_size + 1, len(sentence) - self.input_size + 2):
			print(sentence[i:i+self.input_size])

	def logprob_char(self, sentence:Sentence, c:str):
		return -math.inf

if __name__ == "__main__":
	sentence = Sentence('Hello, my friend')
	vocab = sentence.vocab()
	network = Network(10, vocab)
	network.logprob_sentence(sentence)
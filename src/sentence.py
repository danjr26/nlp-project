
class Sentence:
	def __init__(self, s):
		self.tokens = [token for token in s]

	def __len__(self):
		return len(self.tokens)

	def __str__(self):
		return ''.join(self.tokens)

	def __getitem__(self, key):
		if isinstance(key, int):
			if key < 0:
				return '<BOS>'
			if key >= len(self.tokens):
				return '<EOS>'
			return self.tokens[key]
		
		if isinstance(key, slice):
			return [self[i] for i in range(
				key.start if key.start is not None else 0, 
				key.stop if key.stop is not None else len(self.tokens), 
				key.step if key.step is not None else 1)]
		
		raise ValueError(f'Invalid index into Sentence: {key}')

	def __setitem__(self, key, val):
		if isinstance(key, int):
			if key < 0 or key >= len(self.tokens):
				raise ValueError(f'Invalid index into Sentence: {key}')
			self.tokens[key] = val
			return
		
		if isinstance(key, slice):
			val_iter = iter(val)
			for i in range(
				key.start if key.start is not None else 0, 
				key.stop if key.stop is not None else len(self.tokens), 
				key.step if key.step is not None else 1):
					self.tokens[i] = next(val_iter) 
			return
		
		raise ValueError(f'Invalid index into Sentence: {key}')

	def __iter__(self):
		return iter(self.tokens)

	def vocab(self):
		vocab = set(self.tokens)
		vocab.update({'<BOS>', '<EOS>', '<UNK>'})
		return vocab

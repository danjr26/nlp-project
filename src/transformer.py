import tensorflow as tf
tf.autograph.set_verbosity(0)
tf.get_logger().setLevel('WARNING')
from tensorflow.keras.layers import * 
import sys
import json
import random

def index_to_one_hot(index, alphabet_size):
	one_hot = [0] * alphabet_size
	one_hot[index] = 1
	return one_hot

def one_hot_to_index(one_hot):
	return one_hot.index(1)

def derive_alphabet(corpus):
	alphabet = set(['<UNK>', '<BOS>', '<EOS>'])
	for sentence in corpus:
		alphabet.update(sentence)
	alphabet = list(sorted(alphabet))
	return alphabet

def preprocess(s:list, alphabet_dict:dict, receptive_field:int):
	s = ['<BOS>'] * receptive_field + s + ['<EOS>']
	for i in range(len(s)):
		s[i] = alphabet_dict[s[i]] if s[i] in alphabet_dict else alphabet_dict['<UNK>']
	for i in range(len(s) - receptive_field):
		yield s[i:i+receptive_field], index_to_one_hot(s[i+receptive_field], len(alphabet_dict))

def data_from_corpus(corpus, alphabet_dict, receptive_field):
	batch_size = 64
	xs = []
	ys = []
	for s in corpus:
		for x, y in preprocess(s, alphabet_dict, receptive_field):
			xs.append(x)
			ys.append(y)
			if len(xs) == batch_size:
				batch_x = tf.convert_to_tensor(xs, dtype=tf.uint32)
				batch_y = tf.convert_to_tensor(ys, dtype=tf.float32)
				yield batch_x, batch_y
				xs = []
				ys = []

def load_corpus(filepath, delim='\n#SEP#\n'):
	with open(filepath, encoding='utf-8') as ifs:
		sentences = ifs.read().split(delim)
	sentences = [list(sentence) for sentence in sentences]
	return sentences	

class AttentionPlus(Layer):
	def __init__(self, n_heads, dims):
		super(AttentionPlus, self).__init__()
		self.attn = MultiHeadAttention(n_heads, dims)
		self.add = Add()
		self.norm = LayerNormalization()
		self.dropout = Dropout(0.1)
		self.ffnn = Dense(dims, activation='relu')

	def call(self, input):
		x = input
		residual = x
		x = self.attn(x, x)
		x = self.dropout(x)
		x = self.add([x, residual])
		x = self.norm(x)
		x = self.dropout(x)
		residual = x
		x = self.ffnn(x)
		x = self.dropout(x)
		x = self.add([x, residual])
		x = self.norm(x)
		x = self.dropout(x)
		return x

def create_model(receptive_field, dims, alphabet_size):
	char_input = tf.keras.Input(shape=(receptive_field), dtype=tf.uint32)
	char_embedding = Embedding(alphabet_size, dims)(char_input)

	attn0 = char_embedding
	attn1 = AttentionPlus(4, dims)(attn0)
	attn2 = AttentionPlus(4, dims)(attn1)
	attn3 = AttentionPlus(4, dims)(attn2)
	attn4 = AttentionPlus(4, dims)(attn3)
	attn5 = AttentionPlus(4, dims)(attn4)
	attn6 = AttentionPlus(4, dims)(attn5)
	attn_final = attn6

	conv = Convolution1D(alphabet_size, receptive_field)(attn_final)
	flatten = Flatten()(conv)
	dense = Dense(alphabet_size)(flatten)
	softmax = Softmax()(dense)

	model = tf.keras.Model(inputs=[char_input], outputs=[softmax])
	return model

def load_model(filepath):
	return tf.keras.models.load_model(filepath)

def save_model(model, filepath):
	tf.keras.models.save_model(model, filepath)

def metadata_modify_filepath(filepath):
	return f'{filepath}.metadata.json'

def save_metadata(metadata, filepath):
	with open(metadata_modify_filepath(filepath), 'w', encoding='utf-8') as ofs:
		json.dump(metadata, ofs)

def load_metadata(filepath):
	with open(metadata_modify_filepath(filepath), 'r', encoding='utf-8') as ifs:
		return json.load(ifs)

def prep_model(model, receptive_field):
	model.build((receptive_field))
	model.compile(
		loss=tf.keras.losses.CategoricalCrossentropy(), 
		optimizer=tf.keras.optimizers.Adam(learning_rate=0.0001))
	model.summary()

def train_model(model, data):
	model.fit(data)

def test_model(model, data):
	model.evaluate(data)

def perplexity(y_true, y_pred):
	return 2**tf.keras.losses.categorical_crossentropy(y_true, y_pred)

def parse_params():
	params = {'load_filepath':None, 'save_filepath':None, 'create': False, 'train_filepath':None, 'test_filepath':None, 'epochs': 1}
	args = sys.argv[1:]
	while len(args):
		arg = args.pop(0)
		if arg in ['--create']:
			params['create'] = True
		elif arg in ['--save']:
			params['save_filepath'] = args.pop(0)
		elif arg in ['--load']:
			params['load_filepath'] = args.pop(0)
		elif arg in ['--train']:
			params['train_filepath'] = args.pop(0)
		elif arg in ['--test']:
			params['test_filepath'] = args.pop(0)
		elif arg in ['--epochs']:
			params['epochs'] = int(args.pop(0))
		else:
			print(f'unknown argument: {arg}', file=sys.stderr)
			exit(-1)
	if params['load_filepath'] is not None and params['create']:
		print('incompatible flags: --create, --load', file=sys.stderr)
		exit(-1)
	return params

def main():
	params = parse_params()
	train_corpus = load_corpus(params['train_filepath']) if params['train_filepath'] is not None else None
	test_corpus = load_corpus(params['test_filepath']) if params['test_filepath'] is not None else None
	
	model = None
	metadata = {}
	if params['load_filepath'] is not None:
		model = load_model(params['load_filepath'])
		metadata = load_metadata(params['load_filepath'])
	
	alphabet = metadata.get('alphabet', derive_alphabet(train_corpus))
	receptive_field = metadata.get('receptive_field', 128)
	dims = metadata.get('dims', 64)
	
	alphabet_dict = {c: n for n, c in enumerate(alphabet)}

	if params['create']:
		model = create_model(receptive_field, dims, len(alphabet))

	prep_model(model, receptive_field)

	train_data = None; test_data = None
	if train_corpus:	
		for i in range(params['epochs']):
			random.shuffle(train_corpus)
			train_data = data_from_corpus(train_corpus, alphabet_dict, receptive_field) 
			train_model(model, train_data)
			if test_corpus:
				random.shuffle(test_corpus)
				test_data = data_from_corpus(test_corpus, alphabet_dict, receptive_field)
				test_model(model, test_data)
	elif test_corpus:
		random.shuffle(test_corpus)
		test_data = data_from_corpus(test_corpus, alphabet_dict, receptive_field)
		test_model(model, test_data)

	if params['save_filepath'] is not None:
		metadata = {'alphabet': alphabet, 'receptive_field': receptive_field, 'dims': dims}
		save_model(model, params['save_filepath'])
		save_metadata(metadata, params['save_filepath'])

if __name__ == '__main__':
	main()
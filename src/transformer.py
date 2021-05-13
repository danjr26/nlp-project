import tensorflow as tf
from tensorflow.keras.layers import * 

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
		self.add1 = Add()
		self.relu1 = LeakyReLU()
		self.dropout1 = Dropout(0.5)
		self.ffnn = Dense(dims)
		self.add2 = Add()
		self.relu2 = LeakyReLU()
		self.dropout2 = Dropout(0.5)

	def call(self, input):
		x = input
		residual = x
		x = self.attn(x, x)
		x = self.add1([x, residual])
		x = self.relu1(x)
		x = self.dropout1(x)
		residual = x
		x = self.ffnn(x)
		x = self.add2([x, residual])
		x = self.relu2(x)
		x = self.dropout2(x)
		return x

def create_model(receptive_field, dims, alphabet_size):
	char_input = tf.keras.Input(shape=(receptive_field), dtype=tf.uint32)
	char_embedding = Embedding(alphabet_size, dims)(char_input)

	attn0 = char_embedding
	attn1 = AttentionPlus(4, dims)(attn0)
	attn2 = AttentionPlus(4, dims)(attn1)
	attn3 = AttentionPlus(4, dims)(attn2)
	attn4 = AttentionPlus(4, dims)(attn3)
	attn_final = attn4

	conv = Convolution1D(alphabet_size, receptive_field)(attn_final)
	flatten = Flatten()(conv)
	dense = Dense(alphabet_size)(flatten)
	softmax = Softmax()(dense)

	model = tf.keras.Model(inputs=[char_input], outputs=[softmax])
	return model

def perplexity(y_true, y_pred):
	return 2**tf.keras.losses.categorical_crossentropy(y_true, y_pred)

def main():
	train_corpus = load_corpus("data/train.txt")
	
	alphabet = derive_alphabet(train_corpus)
	alphabet_dict = {c: n for n, c in enumerate(alphabet)}
	receptive_field = 128
	dims = 64

	model = create_model(receptive_field, dims, len(alphabet))
	model.summary()
	model.build((receptive_field))
	model.compile(loss=tf.keras.losses.CategoricalCrossentropy(), optimizer=tf.keras.optimizers.Adam(learning_rate=0.0001), metrics=[perplexity])

	model.fit(data_from_corpus(train_corpus, alphabet_dict, receptive_field))
	tf.keras.models.save_model(model, 'transformer_model')

if __name__ == '__main__':
	main()
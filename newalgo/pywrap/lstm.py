import Pref_pywrap as p
import numpy as np
from pref_utils import readgames, print_data
from collections import Counter
from keras.preprocessing import sequence
from keras.utils import np_utils
from keras.models import Sequential
from keras.engine.topology import Merge
from keras.layers import Dense, Dropout, Activation, Embedding, Flatten, ZeroPadding1D
from keras.layers import LSTM, SimpleRNN, GRU, MaxPooling1D
from keras.layers.convolutional import Convolution1D
from keras.layers.normalization import BatchNormalization
from keras.preprocessing.sequence import pad_sequences
from keras.optimizers import SGD
import json

import tensorflow as tf
tf.python.control_flow_ops = tf


batch_size = 64
epoch_count = 150
hidden_units = 128
num_games = 20000

(data, labels, states) = readgames('game_rec.txt', num_games, type = 'lstm', max_move_number = 7)

print('Build model...')

data = pad_sequences(np.array(data))
#labels = np_utils.to_categorical([la[p.GetCardIndex(card)] for la in labels])
labels = np.array(labels)

print data.shape
model = Sequential()
model.add(LSTM(output_dim=hidden_units, input_shape=data.shape[1:], return_sequences=False, dropout_W = 0.2, dropout_U = 0.2))
model.add(Dense(32, activation = 'sigmoid'))
print model.summary()

model.compile(loss='binary_crossentropy',
              optimizer='adadelta',
              metrics=['accuracy'])

print('Train...')

model.fit(data, labels, batch_size=batch_size, nb_epoch = epoch_count, validation_split = 0.15, show_accuracy = True)
score = model.evaluate(data, labels, batch_size=batch_size)

print_data(model, data, labels, states)

print('Test score:', score)

model_json = {"is_deep" : False, "model" : model.to_json()}
open("model_lstm.json", "w").write(json.dumps(model_json, sort_keys = True, indent = 4))
model.save_weights("model_lstm.h5")

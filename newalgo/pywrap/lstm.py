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
from keras.optimizers import SGD

batch_size = 64
epoch_count = 100
hidden_units = 64

(data, labels, states) = readgames('game_rec.txt', 20000, type = 'lstm')

print('Build model...')

data = np.array(data)
#labels = np_utils.to_categorical([la[p.GetCardIndex(card)] for la in labels])
labels = np.array(labels)

print data.shape
model = Sequential()
model.add(LSTM(output_dim=hidden_units, input_shape=data.shape[1:], return_sequences=False, dropout_W = 0.2, dropout_U = 0.2))
model.add(Dense(32, activation = 'sigmoid'))
print model.summary()

model.compile(loss='binary_crossentropy',
              optimizer='adadelta')

print('Train...')

model.fit(data, labels, batch_size=batch_size, nb_epoch = epoch_count, validation_split = 0.15, show_accuracy = True)
score = model.evaluate(data, labels, batch_size=batch_size)

print_data(model, data, labels, states)

print('Test score:', score)

open("model_lstm.json", "w").write(model.to_json())
model.save_weights("model_lstm.h5")

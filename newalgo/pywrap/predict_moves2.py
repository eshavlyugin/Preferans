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
epoch_count = 200

(data, labels, states) = readgames('game_rec.txt')

print('Build model...')

data = np.array(data)
labels = np_utils.to_categorical(labels)
model = Sequential()
model.add(Dense(32, activation = 'softmax', input_shape = data.shape[1:]))
print model.summary()

model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')

model.fit(data, labels, batch_size=batch_size, nb_epoch = epoch_count, validation_split = 0.15, show_accuracy = True)
score, acc = model.evaluate(data, labels, batch_size=batch_size)

print_data(model, data, labels, states)
print('Test score:', score)
print('Test accuracy:', acc)


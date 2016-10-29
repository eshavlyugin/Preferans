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

batch_size = 32
epoch_count = 100
#epoch_count = 10

(data, labels, states) = readgames('game_rec.txt', min_weight = 0.01, max_games = 18000)

suits = [[d[i:i+8] for i in range(0, len(d), 8)] for d in data]
suits = [[d[i:len(suits):4] for d in suits] for i in range(0,4)]
labels = np_utils.to_categorical(labels)
suits = np.array(suits)
suits = np.array([[np.array(item).transpose() for item in s] for s in suits])

print('Build model...')

models = []
for suit_data in suits:
    print suit_data.shape[1:]
    model = Sequential()
    model.add(Convolution1D(6, 3, border_mode = 'same', activation = 'softmax', input_shape = suit_data.shape[1:]))
    model.add(Convolution1D(6, 3, border_mode = 'same', activation = 'softmax'))
    models.append(model)

merge_model = Sequential()
merge_model.add(Merge(models, mode = 'concat', concat_axis = 1))
merge_model.add(Flatten())
merge_model.add(Dropout(0.2))
merge_model.add(Dense(32, activation = 'sigmoid'))

print merge_model.summary()

merge_model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')

merge_model.fit([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size, nb_epoch=epoch_count, validation_split = 0.15, show_accuracy = True)
score, acc = merge_model.evaluate([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size)

print_data(merge_model, [suits[0], suits[1], suits[2], suits[3]], labels, states)

print('Test score:', score)
print('Test accuracy:', acc)

open("model.json", "w").write(merge_model.to_json())
merge_model.save_weights("model.h5")

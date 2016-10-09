import Pref_pywrap as p
import csv
#import tensorflow as tf
import numpy as np
from random import randint

from keras.preprocessing import sequence
from keras.utils import np_utils
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Embedding
from keras.layers import LSTM, SimpleRNN, GRU
from keras.optimizers import SGD

scores = [0, 0, 0]

max_features =  611
hidden_units = 200
maxlen = 8  # cut texts after this number of words (among top max_features most common words)
batch_size = 32
our_card = '7d'

data = []
labels = []
with open('game_rec.txt') as f:
    gameIdx = 0
    while gameIdx < 15000:
        line = f.readline()
        if not line:
            break
        p1 = p.CardsSet(line.split())
        p2 = p.CardsSet(f.readline().split())
        p3 = p.CardsSet(f.readline().split())
        union = p.CardsSet([]).Add(p1).Add(p2).Add(p3)
        print gameIdx
        gameIdx = gameIdx + 1
        first = int(f.readline())
        gs = p.GameState([p1, p2, p3], first, 'n')
        gs.CloseHands([1,2])
        data.append([])
        idx = randint(0, maxlen - 1)
        for idx, move in enumerate(f.readline().split()):
            if move == our_card or idx == len:
                break
            features = [a for (b,a) in p.CalcFeatures(gs, 0) if b == 'close_cards' or b == 'common_cards'] + [0 if i != p.GetCardIndex(move) else 1 for i in range(0, 32)]
            data[-1].append(np.array(features))
            gs.MakeMove(move)
            
        label = 0
        if our_card in p1:
            label = 0
        elif our_card in p2:
            label = 1
        elif our_card in p3:
            label = 2
        else:
            label = 3
        labels.append(label)
        scores = [a + b for (a,b) in zip(scores, gs.GetScores())]

print 'Final scores:', scores
print 'Total = ', sum(scores)

data = sequence.pad_sequences(data, maxlen=maxlen)
data = np.array(data)
labels = np.array(labels)

X_train = data[0:13500]
X_test = data[13500:]
y_train = labels[0:13500]
y_test = labels[13500:]
Y_train = np_utils.to_categorical(y_train, 4)
Y_test = np_utils.to_categorical(y_test, 4)

print labels
print data.shape, labels.shape

print('Build model...')
model = Sequential()
model.add(LSTM(output_dim=hidden_units, init='uniform', inner_init='uniform',
               forget_bias_init='one', activation='tanh', inner_activation='sigmoid', input_shape=X_train.shape[1:]))
model.add(Dense(4))

model.add(Activation('sigmoid'))

sgd = SGD(lr=0.1, decay=1e-6, momentum=0.9, nesterov=True)
# try using different optimizers and different optimizer configs
model.compile(loss='binary_crossentropy',
              optimizer=sgd,
              metrics=['accuracy'])

print('Train...')
model.fit(X_train, Y_train, batch_size=batch_size, nb_epoch=15,
          validation_data=(X_test, Y_test), show_accuracy = True)
score, acc = model.evaluate(X_test, Y_test,
                            batch_size=batch_size)
print('Test score:', score)
print('Test accuracy:', acc)
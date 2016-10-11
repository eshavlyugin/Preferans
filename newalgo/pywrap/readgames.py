import Pref_pywrap as p
import csv
#import tensorflow as tf
import numpy as np
from random import randint
from collections import Counter
from keras.preprocessing import sequence
from keras.utils import np_utils
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Embedding
from keras.layers import LSTM, SimpleRNN, GRU
from keras.optimizers import SGD

scores = [0, 0, 0]

max_features =  311
hidden_units = 20
maxlen = 6  # cut texts after this number of words (among top max_features most common words)
batch_size = 32
our_card = '7d'

data = []
labels = []
ct = {}
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
        if gameIdx % 200 == 0:
            print gameIdx
        gameIdx = gameIdx + 1
        first = int(f.readline())
        gs = p.GameState([p1, p2, p3], first, 'n')
        gs.CloseHands([1,2])
        features_list = []
        #maxidx = randint(0, maxlen - 1)
        maxidx = maxlen
        #idx = maxlen
        label = 0
        if our_card in p1:
            label = 0
        elif our_card in p2:
            label = 1
        elif our_card in p3:
            label = 2
        else:
            label = 3
        for idx, move in enumerate(f.readline().split()):
            if not move in ct:
                ct[move] = []
            ct[move].append(label)
            if move == our_card or idx == maxidx:
                break
            features = [a for (b,a) in p.CalcFeatures(gs, 0) if b == 'close_cards' or b == 'common_cards'] + [0 if i != p.GetCardIndex(move) else 1 for i in range(0, 32)]
            #fset = [1.0 if card in p1 else 0.0 for card in ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As',
            #                                                '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah',
            #                                                '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac',
            #                                                '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad']]
            #features = fset + [0.0 if i != p.GetCardIndex(move) else 1.0 for i in range(0, 32)]
            features_list.append(np.array(features))
            gs.MakeMove(move)
        if our_card in p1:
            continue
        data.append(features_list)
        labels.append(label)
        scores = [a + b for (a,b) in zip(scores, gs.GetScores())]

print 'Final scores:', scores
print 'Total = ', sum(scores)
print [Counter(c) for c in ct.values()]

data = sequence.pad_sequences(data, maxlen=maxlen)
data = np.array(data)

X_train = data[0:8000]
X_test = data[8000:]
y_train = labels[0:8000]
y_test = labels[8000:]
Y_train = np_utils.to_categorical(y_train)
Y_test = np_utils.to_categorical(y_test)

#
print('Build model...')
model = Sequential()
model.add(LSTM(output_dim=hidden_units, input_shape=X_train.shape[1:], return_sequences=False, dropout_W = 0.25, dropout_U = 0.25))
model.add(Dense(4))
model.add(Activation('sigmoid'))

print model.summary()

#sgd = SGD(lr=0.1, decay=1e-6, momentum=0.9, nesterov=True)
# try using different optimizers and different optimizer configs
model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')
print X_train[0:5]
print Y_train[0:5]
model.fit(X_train, Y_train, batch_size=batch_size, nb_epoch=20,
          validation_data=(X_test, Y_test), show_accuracy = True)
score, acc = model.evaluate(X_test, Y_test,
                            batch_size=batch_size)
print model.predict(X_test)[0:20]
print Y_test[0:20]
print('Test score:', score)
print('Test accuracy:', acc)
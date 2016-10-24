import Pref_pywrap as p
import csv
#import tensorflow as tf
import numpy as np
from random import randint
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
from copy import copy

hidden_units = 5
hidden_dims = 128
batch_size = 64

cardlist = ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah']

data = []
labels = []
states = []
with open('game_rec6.txt') as f:
    gameIdx = 0
    while gameIdx < 60000:
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
	gs2 = copy(gs)
        gs.CloseHands([1,2])
        moveidx = randint(0, 7)
        #moveidx = 0
        moves = f.readline().split()
        label = 0
	isgood = True
	for move in moves:
            if gs.GetMoveNumber() == moveidx and gs.GetCurPlayer() == 0:
                label = p.GetCardIndex(move)
                if label % 8 != 0 and cardlist[label-1] in gs.Hand(0):
                        isgood = False
                if label % 8 != 7 and cardlist[label+1] in gs.Hand(0):
                        isgood = False
                break
	    gs.MakeMove(move)
        if isgood:
            features = [a for (b,a) in p.CalcFeatures(gs, 0) if b == 'close_cards' or b == 'common_cards']
            data.append(features)
            labels.append(label)
            states.append((gs, gs2, moves))

print('Build model...')

data = np.array(data)
labels = np_utils.to_categorical(labels)
model = Sequential()
#model.add(Dropout(0.3, input_shape = data.shape[1:]))
model.add(Dense(32, activation = 'softmax', input_shape = data.shape[1:]))
#merge_model.add(Dense(32, activation = 'softmax'))
#merge_model.add(Dense(32, activation = 'sigmoid'))
print model.summary()

#sgd = SGD(lr=0.02, clipnorm=0.5)
model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')

model.fit(data, labels, batch_size=batch_size, nb_epoch=200, validation_split = 0.15, show_accuracy = True)
score, acc = model.evaluate(data, labels, batch_size=batch_size)

def printhand(h):
        return ' '.join(h.ToArray())

for (probs, label, (gs1, gs2, moves)) in zip(model.predict(data)[0:20], labels[0:20], states[0:20]):
	print probs
	print label
	print printhand(gs1.Hand(0))
	print '******'
	print printhand(gs2.Hand(0))
	print printhand(gs2.Hand(1))
	print printhand(gs2.Hand(2))
	print '******'
	print ' '.join(moves)
#	print printhand(state.Hand(1))
#	print printhand(state.Hand(2))
	print '---------------------------------'

print('Test score:', score)
print('Test accuracy:', acc)


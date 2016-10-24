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
batch_size = 32
epoch_count = 200
#epoch_count = 10
data = []
labels = []
states = []
cardlist = ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah']

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
        moveidx = randint(0, 2)
        #moveidx = 
	#moveidx = 2
        moves = f.readline().split()
        label = 0
	isgood = True
	for move in moves:
            if gs.GetMoveNumber() == moveidx and gs.GetCurPlayer() == 0:
#                label = p.EncodeMoveIndex(gs, move)
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

suits = [[d[i:i+8] for i in range(0, len(d), 8)] for d in data]
suits = [[d[i:len(suits):4] for d in suits] for i in range(0,4)]
labels = np_utils.to_categorical(labels)
suits = np.array(suits)
suits = np.array([[np.array(item).transpose() for item in s] for s in suits])
#
print('Build model...')

models = []
for suit_data in suits:
    print suit_data.shape[1:]
    model = Sequential()
    model.add(Convolution1D(6, 3, border_mode = 'same', activation = 'softmax', input_shape = suit_data.shape[1:]))
    model.add(Convolution1D(6, 3, border_mode = 'same', activation = 'softmax'))
    #    model.add(Activation('sigmoid'))
    #model.add(Convolution1D(7, 2, border_mode = 'valid', activation = 'softmax'))
    #model.add(Convolution1D(7, 3, border_mode = 'same', activation = 'softmax'))
    #model.add(MaxPooling1D(pool_length = 2))
#    model.add(Dropout(0.04))
    models.append(model)

merge_model = Sequential()
merge_model.add(Merge(models, mode = 'concat', concat_axis = 1))
merge_model.add(Flatten())
#data = np.array(data)
merge_model.add(Dropout(0.2))
merge_model.add(Dense(32, activation = 'sigmoid'))
#merge_model.add(Dense(32, activation = 'sigmoid'))
print merge_model.summary()

#sgd = SGD(lr=0.02, clipnorm=0.5)
merge_model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')

#print suits[0].shape
#merge_model.fit(data, labels, batch_size=batch_size, nb_epoch=160, validation_split = 0.15, show_accuracy = True)
merge_model.fit([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size, nb_epoch=epoch_count, validation_split = 0.15, show_accuracy = True)
score, acc = merge_model.evaluate([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size)

def printhand(h):
        return ' '.join(h.ToArray())

for (probs, label, (gs1, gs2, moves)) in zip(merge_model.predict([suits[0], suits[1], suits[2], suits[3]])[0:20], labels[0:20], states[0:20]):
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


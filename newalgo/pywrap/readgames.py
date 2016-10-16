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
from copy import copy

scores = [0, 0, 0]

max_features =  311
hidden_units = 5
maxlen = 30
batch_size = 32
our_card = 'Ks'

data = []
labels = []
states = []
with open('game_rec.txt') as f:
    gameIdx = 0
    while gameIdx < 45000:
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
        features_list = []
        #maxidx = randint(0, maxlen - 1)
        maxidx = maxlen
        #idx = maxlen
        label = 0
#        if our_card in p1:
#            label = 0
        if our_card in p2:
            label = 0
        elif our_card in p3:
            label = 1
#        else:
 #           label = 3
	moves = f.readline().split()
	our_idx = -1
	if our_card in moves:
		our_idx = moves.index(our_card)
	moves2 = []
	if our_idx > 7:
		ll = randint(1,our_idx-7)
		moves2 = moves[0:our_idx-ll]
	if sum([1 if our_card[1] in move else 0 for move in moves2]) < 2:
		moves2 = []
	if moves2:
	        for idx, move in enumerate(moves2):
	            features = [a for (b,a) in p.CalcFeatures(gs, 0) if b == 'close_cards' or b == 'common_cards'] + [0 if i != p.GetCardIndex(move) else 1 for i in range(0, 32)]
	            #fset = [1.0 if card in p1 else 0.0 for card in ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As',
	            #                                                '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah',
	            #                                                '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac',
	            #                                                '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad']]
	            #features = fset + [0.0 if i != p.GetCardIndex(move) else 1.0 for i in range(0, 32)]
        	    features_list.append(np.array(features))
	            gs.MakeMove(move)
        if (not our_card in p1) and (our_card in union) and moves2:
            data.append(features_list)
	    labels.append(label)
            scores = [a + b for (a,b) in zip(scores, gs.GetScores())]
	    states.append((gs, gs2, moves2))

print 'Final scores:', scores
print 'Total = ', sum(scores)

data = sequence.pad_sequences(data, maxlen=maxlen)
data = np.array(data)

X_train = data[0:10000]
X_test = data[10000:]
y_train = labels[0:10000]
y_test = labels[10000:]
Y_train = np_utils.to_categorical(y_train)
Y_test = np_utils.to_categorical(y_test)

#
print('Build model...')
model = Sequential()
model.add(LSTM(output_dim=hidden_units, input_shape=X_train.shape[1:], return_sequences=False, dropout_W = 0.35, dropout_U = 0.35))
model.add(Dense(2))
model.add(Activation('softmax'))

print model.summary()

#sgd = SGD(lr=0.1, decay=1e-6, momentum=0.9, nesterov=True)
# try using different optimizers and different optimizer configs
model.compile(loss='categorical_crossentropy',
              optimizer='adam',
              metrics=['accuracy'])

print('Train...')
print X_train[0:5]
print Y_train[0:5]
model.fit(X_train, Y_train, batch_size=batch_size, nb_epoch=30,
          validation_data=(X_test, Y_test), show_accuracy = True)
score, acc = model.evaluate(X_test, Y_test,
                            batch_size=batch_size)
print model.predict(X_train)[0:30]
print Y_train[0:30]

def printhand(h):
	res = ''
	for card in h:
		res += ' ' + str(card)
	return res

for (probs, label, (gs1, gs2, moves)) in zip(model.predict(X_train)[0:30], Y_train[0:30], states[0:30]):
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

with open('model.cpp', 'w') as f:
	for e in zip(model.layers[0].trainable_weights, model.layers[0].get_weights()):
		f.write(str(e[0]))
		f.write(str(e[1]))

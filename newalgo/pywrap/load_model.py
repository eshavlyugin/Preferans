import Pref_pywrap as p
import numpy as np
from pref_utils import readgames, print_data
from collections import Counter
from keras.models import model_from_json
from keras.utils import np_utils

batch_size = 32
model_name = "model_lstm"
model_type = "lstm" if model_name == "model_lstm" else "move"
(data, labels, states) = readgames('game_rec9.txt', min_weight = 0.13, type = model_type)

model = model_from_json(open(model_name + ".json", "r").read())
model.load_weights(model_name + ".h5")
if model_type == "move":
	suits = [[d[i:i+8] for i in range(0, len(d), 8)] for d in data]
	suits = [[d[i:len(suits):4] for d in suits] for i in range(0,4)]
	labels = np_utils.to_categorical(labels)
	suits = np.array(suits)
	suits = np.array([[np.array(item).transpose() for item in s] for s in suits])
	model.compile(loss='categorical_crossentropy',
              optimizer='adam')
else:
	model.compile(loss='binary_crossentropy',
              optimizer='adam')

model.summary()

if model_name in set(["model", "model_deep"]):
	score = model.evaluate([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size)
	print_data(model, [suits[0], suits[1], suits[2], suits[3]], labels, states)
elif model_name == "model2":
	score = model.evaluate(data, labels, batch_size=batch_size)
	print_data(model, data, labels, states)
elif model_name == "model_lstm":
	score = model.evaluate(data, labels, batch_size=batch_size)
	print_data(model, data, labels, states)
print('Test score:', score)
#print('Test accuracy:', acc)


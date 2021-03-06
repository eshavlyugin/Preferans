import Pref_pywrap as p
import numpy as np
from pref_utils import readgames, print_data
from collections import Counter
from keras.models import model_from_json
from keras.utils import np_utils

import tensorflow as tf
tf.python.control_flow_ops = tf

batch_size = 32
model_name = "model2"
model_type = "lstm" if model_name == "model_lstm" else "move"
(data, labels, states) = readgames('game_rec9.txt', min_weight = 0.08, type = model_type)

model = model_from_json(open(model_name + ".json", "r").read())
model.load_weights(model_name + ".h5")
if model_type == "move":
	labels = np_utils.to_categorical(labels)
	model.compile(loss='categorical_crossentropy',
                      optimizer='adam',
                      metrics=["accuracy"])
else:
	model.compile(loss='binary_crossentropy',
                      optimizer='adam',
                      metrics=["accuracy"])

model.summary()

acc = None
if model_name in set(["model", "model_deep"]):
	score, acc = model.evaluate([suits[0], suits[1], suits[2], suits[3]], labels, batch_size=batch_size)
	print_data(model, [suits[0], suits[1], suits[2], suits[3]], labels, states)
elif model_name == "model2":
	data = np.array(data)
	score, acc = model.evaluate(data, labels, batch_size=batch_size)
	print_data(model, data, labels, states)
elif model_name == "model_lstm":
	score = model.evaluate(data, labels, batch_size=batch_size)
	print_data(model, data, labels, states)
print('Test score:', score)
if acc:
        print('Test accuracy:', acc)


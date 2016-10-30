import sys

# TODO: find alternative solution workaround
#sys.argv = ["./TestGame"]

import numpy as np
from keras.models import model_from_json
from keras.utils import np_utils

import tensorflow as tf
tf.python.control_flow_ops = tf

class KerasModel(object):
        model_ = None
        
        def __init__(self, model_name):
                self.model_ = model_from_json(open(model_name + ".json", "r").read())
                self.model_.load_weights(model_name + ".h5")

        def predict(self, data):
                print "hello from predict!"
                print data
                return self.model_.predict(data)


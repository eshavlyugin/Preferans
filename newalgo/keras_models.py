import sys
import json
import numpy as np
from keras.models import model_from_json
from keras.utils import np_utils

import tensorflow as tf
tf.python.control_flow_ops = tf

class KerasModel(object):
        model_ = None
        
        def __init__(self, model_name):
                json_data = json.loads(open(model_name + ".json", "r").read())
                self.is_deep_ = json_data["is_deep"]
                self.time_steps_ = json_data["time_steps"]
                self.model_ = model_from_json(json_data["model"])
                self.model_.load_weights(model_name + ".h5")

        def is_layered(self):
                return self.is_deep_

        def time_steps(self):
                return self.time_steps_
        
        def predict(self, data):
                return np.array(self.model_.predict(data), dtype=np.float32)


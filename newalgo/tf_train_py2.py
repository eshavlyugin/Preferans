import csv
import argparse
import sklearn
import sklearn.ensemble
import numpy
from sklearn.metrics import accuracy_score
from itertools import count

batch_size = 512
num_steps = 200000
num_hidden1 = 50
num_hidden2 = 1

def prepare_train_data(data, label_names, label_name, feature_set, num_labels, use_regression):
    if use_regression:
        return prepare_train_data_regression(data, label_names, label_name, feature_set, num_labels)
    else:
        return prepare_train_data_label(data, label_names, label_name, feature_set, num_labels)

def prepare_train_data_regression(data, label_names, label_name, feature_set, num_labels):
    req_indexes = set([])
    val_indexes = set([])
    
    for (idx, val) in enumerate(label_names):
        if val in feature_set:
            req_indexes.add(idx)
        if val == label_name:
            val_indexes.add(idx)
    assert len(val_indexes) == num_labels, "value indexes count doesn't match the labels count"
    
    labels = np.array([[float(entry[label_idx]) for label_idx in val_indexes] for entry in data], dtype=np.float32)
    dataset = np.array([[float(val) for (idx, val) in enumerate(entry) if idx in req_indexes] for entry in data], dtype=np.float32)
    
    return (labels, dataset)
   
def prepare_train_data_label(data, label_names, label_name, feature_set, num_labels):
    label_idx = -1
    assert not label_name in feature_set, "prediction labels couldn't be in feature set!"

    req_indexes = set([])
    reward_indexes = [];
    
    for (idx, val) in enumerate(label_names):
        if val in feature_set:
            req_indexes.add(idx)
          
        if val == label_name:
            assert label_idx == -1, "label " + label_name + " is not unique"
            label_idx = idx
            
        if val == "move_reward":
            reward_indexes.append(idx)
        
    assert label_idx != -1, "label " + label_name + " not found"

    reward_indexes_set = set(reward_indexes)
    
    from collections import Counter
    print('Counter:', Counter([entry[label_idx] for entry in data]))
    labels = np.array([[1.0 if i == entry[label_idx] else 0.0 for i in range(0, num_labels)] for entry in data], dtype=np.float32)
    #labels = np.array([[entry[reward_indexes[i]] / entry[reward_indexes[int(entry[label_idx])]] for i in range(0, num_labels)] for entry in data], dtype=np.float32)
    dataset = np.array([[float(val) for (idx, val) in enumerate(entry) if idx in req_indexes] for entry in data], dtype=np.float32)
    #weights = np.array([entry[reward_indexes[int(entry[label_idx])]] - 1.0 / 32 * sum([val if idx in reward_indexes_set else 0.0 for (idx, val) in enumerate(entry)]) for entry in data], dtype=np.float32)
    #num_labels = np.array([int(entry[label_idx]) for entry in data], dtype=np.int)
    
    return (labels, dataset)

def correct(predictions, labels):
    return 1.0 * np.sum(np.argmax(predictions, 1) == np.argmax(labels, 1))

def accuracy(predictions, labels):
    return (1.0 * np.sum(np.argmax(predictions, 1) == np.argmax(labels, 1)) / predictions.shape[0])

#def accuracy(predictions, labels):
#    return (1.0 * np.sum((predictions > 0.5) == labels)) / predictions.shape[0] / predictions.shape[1]

def load_model_data(model_file):

    with open(model_file,'r') as tsv:
        label_names = tsv.readline().strip().split('\t');
        lines = [[float(part) for part in line.strip().split('\t')] for line in tsv]

    total = len(lines)
    train_subset = int(total * 0.8)
    valid_subset = int(total * 0.1)
    test_subset = total - valid_subset - train_subset

    print 'Size = ', train_subset, valid_subset, test_subset
    all_dataset = lines
    train_dataset = np.array(all_dataset[:train_subset], dtype=np.float32)
    valid_dataset = np.array(all_dataset[train_subset:train_subset+valid_subset], dtype=np.float32)
    test_dataset = np.array(all_dataset[train_subset+valid_subset:], dtype=np.float32)

    return (label_names, train_dataset, valid_dataset, test_dataset)

def singlelayer_perceptron(data, weights, biases):
    return tf.matmul(data, weights) + biases

def three_layer_perceptron(data, weights, biases, weights2, biases2, weights3, drop):
    # Hidden layer with RELU activation
    if drop:
        data = tf.nn.dropout(data, 0.9)
    layer_1 = tf.add(tf.matmul(data, weights), biases)
    layer_1 = tf.nn.sigmoid(layer_1)
    if drop:
        layer_1 = tf.nn.dropout(layer_1, 0.7)
    # Hidden layer with RELU activation
    layer_2 = tf.add(tf.matmul(layer_1, weights2), biases2)
    layer_2 = tf.nn.sigmoid(layer_2)
    if drop:
        layer_2 = tf.nn.dropout(layer_2, 0.7)
    layer_3 = tf.matmul(layer_2, weights3)
    return layer_3

def two_layer_perceptron(data, weights, biases, weights2, drop):
    # Hidden layer with RELU activation
    #data = tf.nn.dropout(data, 0.8)
    if drop:
        data = tf.nn.dropout(data, 0.9)
    layer_1 = tf.add(tf.matmul(data, weights), biases)
    layer_1 = tf.nn.sigmoid(layer_1)
    if drop:
        layer_1 = tf.nn.dropout(layer_1, 0.7)
    
    #layer_1 = tf.nn.dropout(layer_1, 0.8)
    # Hidden layer with RELU activation
    layer_2 = tf.matmul(layer_1, weights2)
    return layer_2

def multilayer_perceptron(data, weights, biases, weights2, biases2, weights3, drop):
   # return singlelayer_perceptron(data, weights, biases)
    return two_layer_perceptron(data, weights, biases, weights2, drop)
  #  return three_layer_perceptron(data, weights, biases, weights2, biases2, weights3, drop)

def train_predict_model(model_data, label_name, feature_set, num_labels, use_regression):
    label_names, train_dataset, valid_dataset, test_dataset = model_data
    train_labels, train_dataset = prepare_train_data(train_dataset, label_names, label_name, feature_set, num_labels, use_regression)
    valid_labels, valid_dataset = prepare_train_data(valid_dataset, label_names, label_name, feature_set, num_labels, use_regression)
    test_labels, test_dataset = prepare_train_data(test_dataset, label_names, label_name, feature_set, num_labels, use_regression)
    num_features = len(train_dataset[0])
    #clf = sklearn.ensemble.RandomForestClassifier(verbose=1, n_estimators=500)
    #clf.fit(train_dataset, train_num_labels)
    #print(clf.predict(test_dataset))
    #print(accuracy_score(test_num_labels,clf.predict(test_dataset)))
    #return
#sum(test_num_labels == clf.predict(test_labels))
    
    print('Processing: ', label_name, num_features, num_labels)
    graph = tf.Graph()
    with graph.as_default():
        # Input data.
        # Load the training, validation and test data into constants that are
        # attached to the graph.
        tf_train_dataset = tf.placeholder(tf.float32, shape=(batch_size, num_features))    
        tf_train_labels = tf.placeholder(tf.float32, shape=(batch_size, num_labels))
        #tf_valid_labels = tf.placeholder(tf.float32, shape=(batch_size, num_labels))
        tf_train_dataset_const = tf.constant(numpy.concatenate((train_dataset, test_dataset, valid_dataset), axis = 0))
        tf_train_labels_const = tf.constant(numpy.concatenate((train_labels, test_labels, valid_labels), axis = 0))
        tf_valid_dataset = tf.constant(valid_dataset)
        tf_valid_labels = tf.constant(valid_labels)
        tf_test_dataset = tf.constant(test_dataset)
        
    #    print num_features, num_hidden, num_labels
        
        weights = tf.Variable(tf.truncated_normal([num_features, num_hidden1]))
        biases = tf.Variable(tf.zeros([num_hidden1]))
        weights2 = tf.Variable(tf.truncated_normal([num_hidden1, num_hidden2]))
        biases2 = tf.Variable(tf.zeros([num_hidden2]))
        weights3 = tf.Variable(tf.truncated_normal([num_hidden2, num_labels]))
        #logits2 = singlelayer_perceptron(tf_train_dataset, weights, biases)
        logits2 = multilayer_perceptron(tf_train_dataset, weights, biases, weights2, biases2, weights3, True)
        logits_all = multilayer_perceptron(tf_train_dataset_const, weights, biases, weights2, biases2, weights3, False)
        logits_valid = multilayer_perceptron(tf_valid_dataset, weights, biases, weights2, biases2, weights3, False)
        loss = tf.reduce_mean(tf.square(logits2 - tf_train_labels))
        loss_all = tf.reduce_mean(tf.square(logits_all - tf_train_labels_const))
        loss_valid = tf.reduce_mean(tf.square(logits_valid - tf_valid_labels))
        #loss = tf.reduce_mean(tf.mul(tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits2, tf_train_labels), 0), tf_train_weights))

     #   loss = tf.reduce_mean(tf.neg(tf.mul(tf_train_labels, tf.nn.softmax(logits=logits2))))
    #    loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits2, tf_train_labels))
        
        #regularizers = (tf.nn.l2_loss(weights) + tf.nn.l2_loss(biases))
        #loss += 1e-5 * regularizers
        # Optimizer.
        # We are going to find the minimum of this loss using gradient descent.
        #optimizer = tf.train.GradientDescentOptimizer(0.5).minimize(loss)
        optimizer = tf.train.AdamOptimizer(learning_rate=0.001).minimize(loss)

        # Predictions for the training, validation, and test data.
        # These are not part of training, but merely here so that we can report
        # accuracy figures as we train.
        train_prediction = tf.nn.softmax(logits2)
        #valid_prediction = tf.nn.softmax(singlelayer_perceptron(tf_valid_dataset, weights, biases))
        #test_prediction = tf.nn.softmax(singlelayer_perceptron(tf_test_dataset, weights, biases))
        #train_all_prediction = tf.nn.softmax(singlelayer_perceptron(tf_train_dataset_const, weights, biases))
        #valid_prediction = tf.reduce_mean(tf.square(tf.nn.softmax(multilayer_perceptron(tf_valid_dataset, weights, biases, weights2, biases2)), tf_valid_labels))
        test_prediction = tf.nn.softmax(multilayer_perceptron(tf_test_dataset, weights, biases, weights2, biases2, weights3, False))
        valid_prediction = tf.nn.softmax(multilayer_perceptron(tf_valid_dataset, weights, biases, weights2, biases2, weights3, False))
        #train_all_prediction = tf.nn.softmax(multilayer_perceptron(tf_train_dataset_const, weights, biases, weights2, biases2))
        
    with tf.Session(graph=graph, config=tf.ConfigProto(allow_soft_placement=True, log_device_placement=True)) as session:
        # This is a one-time operation which ensures the parameters get initialized as
        # we described in the graph: random weights for the matrix, zeros for the
        # biases.
        tf.initialize_all_variables().run()
        print('Initialized')
        for step in range(num_steps):
            # Run the computations. We tell .run() that we want to run the optimizer,
            # and get the loss value and the training predictions returned as numpy
            # arrays.
            offset = (step * batch_size) % (train_labels.shape[0] - batch_size)
            # Generate a minibatch.
            batch_data = train_dataset[offset:(offset + batch_size), :]
            batch_labels = train_labels[offset:(offset + batch_size), :]
            feed_dict = {tf_train_dataset : batch_data, tf_train_labels : batch_labels}
            _, l, predictions = session.run([optimizer, loss, train_prediction], feed_dict = feed_dict)
            if (step % 100 == 0):
                print('Loss at step %d: %f' % (step, l))
                print('Validation loss: %f' % loss_valid.eval())
                #print('Train accuracy: %f' % accuracy(predictions, batch_labels))
                #print('Training accuracy: %.1f%%' % accuracy(predictions, batch_labels))
                # Calling .eval() on valid_prediction is basically like calling run(), but
                # just to get that one numpy array. Note that it recomputes all its graph
                # dependencies.
                #print('Validation accuracy: %f' % accuracy(valid_prediction.eval(), valid_labels))
                #print('Test accuracy: %f' % accuracy(test_prediction.eval(), test_labels))
        
        weights_eval = weights.eval()
        biases_eval = biases.eval()
        weights2_eval = weights2.eval()
        biases2_eval = biases2.eval()
        weights3_eval = weights3.eval()
        
        trained_weights_file = label_name + ".tsv"
        with open(trained_weights_file, 'w') as f:
            f.write('2_layer_nn\n')
            f.write(str(num_features) + " " + str(num_hidden1) + " " + "1" + "\n")
            f.write('\t'.join([str(i) for i in biases_eval]) + '\n')
            for row in weights_eval:
                f.write('\t'.join([str(i) for i in row]) + '\n')
            for row in weights2_eval:
                f.write('\t'.join([str(i) for i in row]) + '\n')
        with open("model_error.txt", 'w') as f:
            f.write('%f\n' % (loss_all.eval()))
            
if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Tensorflow model trainer', add_help=True)
    parser.add_argument('--play_open', action='store_true', help='do train move predictor model')
    parser.add_argument('--play_close', action='store_true', help='do train move predictor model')
    parser.add_argument('--prob', action='store_true', help='do train probabilities predictor model')
    args = parser.parse_args()
  
    if args.play_open or args.prob or args.play_close:
        import tensorflow as tf
        import numpy as np

        data = load_model_data('model.tsv')
        if args.play_open:
            train_predict_model(data, 'expected_score', set(['open_cards', 'common_cards']), 1, True)
            #train_predict_model(data, 'expected_score_3', set(['open_cards', 'common_cards']), 2, False)
            #train_predict_model(data, 'expected_score_6', set(['open_cards', 'common_cards']), 2, False)
        if args.play_close:
            train_predict_model(data, 'correct_move_close', set(['close_cards', 'common_cards']), 32, False)
        if args.prob:
            for i in range(0, 32):
                train_predict_model(data, 'card' + str(i), set(['close_cards', 'common_cards' 'move']), 4, False)


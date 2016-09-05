import csv
import tensorflow as tf
import numpy as np

train_subset = 80000
valid_subset = 10000
test_subset = 10000

labels_count = 33
num_labels = 32
batch_size = 256

def prepare_train_labels(labels):
  answer = [[1.0 if i == label[0] else 0.0 for i in range(0, num_labels)]
            + [1.0 if i == lab else 0.0 for lab in label[1:] for i in range(0,4)] for label in labels]
  return [labels[0:32] for labels in answer]
  
def accuracy(predictions, labels):
  return (100.0 * np.sum(np.argmax(predictions, 1) == np.argmax(labels, 1)) / predictions.shape[0])

def update_probabilities_model(model_file):
  pass

def train_predict_model(model_file, trained_weights_file):
  with open(model_file,'r') as tsv:
    lines = [[float(part) for part in line.strip().split('\t')] for line in tsv]

  num_features = len(lines[0]) - labels_count
  
  train_labels = [line[num_features:] for line in lines]
  train_labels = prepare_train_labels(train_labels)
  train_dataset = [line[0:num_features-1] for line in lines]
  valid_labels = np.array(train_labels[train_subset:train_subset+valid_subset], dtype=np.float32)
  test_labels = np.array(train_labels[train_subset+valid_subset:], dtype=np.float32)
  valid_dataset = np.array(train_dataset[train_subset:train_subset+valid_subset], dtype=np.float32)
  test_dataset = np.array(train_dataset[train_subset+valid_subset:], dtype=np.float32)
  train_labels = np.array(train_labels[:train_subset], dtype=np.float32)
  train_dataset = np.array(train_dataset[:train_subset], dtype=np.float32)

  num_features = len(train_dataset[0])
  num_labels = len(train_labels[0])
  
  graph = tf.Graph()
  with graph.as_default():
    # Input data.
    # Load the training, validation and test data into constants that are
    # attached to the graph.
    tf_train_dataset = tf.constant(train_dataset)
    tf_train_labels = tf.constant(train_labels)
    tf_valid_dataset = tf.constant(valid_dataset)
    tf_test_dataset = tf.constant(test_dataset)
    
    weights = tf.Variable(tf.truncated_normal([num_features, num_labels]))
    biases = tf.Variable(tf.zeros([num_labels]))
    
    logits = tf.matmul(tf_train_dataset, weights) + biases
    loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits, tf_train_labels))
    
    # Optimizer.
    # We are going to find the minimum of this loss using gradient descent.
    optimizer = tf.train.GradientDescentOptimizer(0.5).minimize(loss)
    
    # Predictions for the training, validation, and test data.
    # These are not part of training, but merely here so that we can report
    # accuracy figures as we train.
    train_prediction = tf.nn.softmax(logits)
    valid_prediction = tf.nn.softmax(tf.matmul(tf_valid_dataset, weights) + biases)
    test_prediction = tf.nn.softmax(tf.matmul(tf_test_dataset, weights) + biases)
    
    num_steps = 4001

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
        print('Training accuracy: %.1f%%' % accuracy(predictions, batch_labels))
        # Calling .eval() on valid_prediction is basically like calling run(), but
        # just to get that one numpy array. Note that it recomputes all its graph
        # dependencies.
        print('Validation accuracy: %.1f%%' % accuracy(valid_prediction.eval(), valid_labels))
        print('Test accuracy: %.1f%%' % accuracy(test_prediction.eval(), test_labels))

    weights_eval = weights.eval()
    biases_eval = biases.eval()

    with open(trained_weights_file, 'w') as f:
      f.write('\t'.join([str(i) for i in biases_eval]) + '\n')
      for row in weights_eval:
        f.write('\t'.join([str(i) for i in row]) + '\n')

train_predict_model('model.tsv', 'weights_trained.tsv')

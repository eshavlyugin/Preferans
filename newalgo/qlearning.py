import subprocess
import argparse
import os
import shutil
import random

# input [(folder, idx), ...], current idx
def make_player_string(players, cur_idx, q):
    res = ""
    for (folder, idx) in players:
        if res:
            res = res + ';'
        res = res + folder + "," + str(q ** (cur_idx - idx))
    return res

def generate_training_data(player1, player2, player3, num_samples, play_per_sample):
    cmd_line = ["./TrainModel"]
    cmd_line.append("--play1")
    cmd_line.append(player1)
    cmd_line.append("--play2")
    cmd_line.append(player2)
    cmd_line.append("--play3")
    cmd_line.append(player3)
    cmd_line.append("--num-samples")
    cmd_line.append(str(num_samples))
    cmd_line.append("--simulations-per-move")
    cmd_line.append(str(play_per_sample))
    print(' '.join(cmd_line))
    proc = subprocess.Popen(" ".join(cmd_line), shell=True)
    proc.wait()

def do_single_step(work_dir, cur_folder, cur_prob_folder, next_folder, opponent, gen_card_prob = False, num_samples = 20000, play_per_sample = 20):
    os.makedirs(next_folder, exist_ok=True)
    if gen_card_prob:
        prob_folder = next_folder
        generate_training_data("{cur_folder}:{cur_prob_folder}:random".format(cur_folder=cur_folder, cur_prob_folder=cur_prob_folder), opponent, opponent, num_samples, play_per_sample)
        train_model_tf(prob = True)
        for i in range(0, 32):
            shutil.copy(os.path.join(work_dir, "card{i}.tsv".format(i=i)), os.path.join(work_dir, next_folder, "card{i}.tsv".format(i=i)))
    else:
        prob_folder = "random"
    next_player_str = "{cur_folder}:{cur_prob_folder}:{prob_folder}".format(cur_folder=cur_folder, cur_prob_folder=cur_prob_folder, next_folder=next_folder, prob_folder = prob_folder)
    generate_training_data(next_player_str, opponent, opponent, num_samples, play_per_sample)
    train_model_tf(play = True)
    shutil.copy(os.path.join(work_dir, "correct_move.tsv"), os.path.join(work_dir, next_folder, "correct_move.tsv"))
    return "{play}:{prob}:random".format(play = next_folder, prob = prob_folder)

def train_model_tf(play = False, prob = False):
    cmd_line = ["python2 tf_train_py2.py"]
    if play:
        cmd_line.append("--play")
    if prob:
        cmd_line.append("--prob")
    proc = subprocess.Popen(" ".join(cmd_line), shell=True)
    proc.wait()
       
def do_learning(Q = 0.8, epochs = 10, players_at_once = 0):
    rng = random.Random()
    players = ["random:random:random"]
    for epoch in range(0, epochs):
        idxs = set([len(players) - 1])
        cur_players = [(players[-1], len(players) - 1)]
        print(players)
        for i in range(0, min(players_at_once, len(cur_players) - 1)):
            idx = rng.randint(0, len(cur_players) - 1)
            if not idx in idxs:
                idxs.add(idx)
                cur_players.append((players[idx], idx))
        player_str = make_player_string(cur_players, epoch + 1, Q)
        current_folder = "models{i}".format(i = epoch - 1)
        next_folder = "models{i}".format(i = epoch)
        if epoch == 0:
            next_player = do_single_step(os.getcwd(), "random", "random", next_folder, player_str, False, num_samples = 20000, play_per_sample = 30)
        elif epoch == 1:
            next_player = do_single_step(os.getcwd(), current_folder, "random", next_folder, player_str, True, num_samples = 20000, play_per_sample = 5)
        else:
            next_player = do_single_step(os.getcwd(), current_folder, current_folder, next_folder, player_str, True, num_samples = 20000, play_per_sample = 5)
        players.append(next_player)
        
if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Q-learning trainer', add_help=True)
    parser.add_argument('--Q', help='A parameter for qlearning model')
    parser.add_argument('--epochs', help='number of epochs to train model')
    args = parser.parse_args()
    do_learning(Q = float(args.Q), epochs = int(args.epochs))

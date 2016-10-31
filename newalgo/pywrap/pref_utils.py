import Pref_pywrap as p
from random import randint
from copy import copy

cardlist_ = ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah']

def printhand(h):
        return ' '.join(h.ToArray())

def _init_data(deep):
	if deep:
		return [[],[],[],[]]
	else:
		return []

def _join_data(cur, item, deep, singleitem):
	if deep:
		for c, i in zip(cur, item):
			if singleitem:
				c += [i]
			else:
				c += i
	else:
		if singleitem:
			cur += [item]
		else:
			cur += item

def readgame_lstm(gs, moves, max_move_number, deep):
	data = _init_data(deep)
	gs2 = copy(gs)
	features = [_init_data(deep), _init_data(deep), _init_data(deep)] # one vector for each player
	states = []
        max_move = randint(1, max_move_number)
	for move in moves:
		if gs2.GetMoveNumber() > max_move:
			break
		curpl = gs2.GetCurPlayer()
		features[curpl] += [p.CalcFeatures(gs2, curpl, move, 'pos_predict', deep)]
		gs2.MakeMove(move)
	labels = [[1.0 if c in gs2.Hand(pl) else 0.0 for c in cardlist_] for pl in range(0,3)]
	return (features, labels, [(gs2.Hand(i), copy(gs), moves) for i in range(0,3)])

def readgame_move(gs, moves, weights, max_move_number, min_weight, deep):
	data = _init_data(deep)
	labels = []
	states = []
        for (move, weight) in zip(moves, weights):
 	 	if gs.GetMoveNumber() > max_move_number:
			break
		label = p.GetCardIndex(move)
		hand = gs.GetCurPlayer()
				
		if weight > min_weight:
			gs2 = copy(gs)
			gs2.CloseHands([i for i in range(0,3) if i != hand])
                        features = p.CalcFeatures(gs2, hand, move, 'playing', deep)
			_join_data(data, features, deep, True)
                        labels.append(label)
                        states.append((gs2.Hand(i), copy(gs), moves))
		gs.MakeMove(move)
	return (data, labels, states)

def readgames(filename, max_games = None, min_weight = 0.08, max_move_number = 5, type = 'move', deep = False):
	data = _init_data(deep)
	labels = []
	states = []
	with open(filename) as f:
		gameIdx = 0
		while not max_games or gameIdx < max_games:
		    gameIdx = gameIdx + 1
		    line = f.readline()
		    if not line:
		        break
		    p1 = p.CardsSet(line.split())
		    p2 = p.CardsSet(f.readline().split())
		    p3 = p.CardsSet(f.readline().split())
		    first = int(f.readline())
	            moves = f.readline().split()
        	    move_weights = [float(w) for w in f.readline().split()]
		    gs = p.GameState([p1, p2, p3], first, 'n')
		    if gameIdx % 200 == 0:
		        print gameIdx
		    if type == 'move':
			d, l, s = readgame_move(copy(gs), moves, move_weights, max_move_number, min_weight, deep)
		    elif type == 'lstm':
			d, l, s = readgame_lstm(copy(gs), moves, max_move_number, deep)
		    _join_data(data, d, deep, False)
		    labels += l
		    states += s
	return (data, labels, states)


def print_data(model, data, labels, states):
	for (probs, label, (h_cur, gs2, moves)) in zip(model.predict(data)[0:20], labels[0:20], states[0:20]):
		print probs
		print label
		print printhand(h_cur)
		print '******'
		print printhand(gs2.Hand(0))
		print printhand(gs2.Hand(1))
		print printhand(gs2.Hand(2))
		print '******'
		print ' '.join(moves)
		print '---------------------------------'


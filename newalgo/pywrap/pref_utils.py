import Pref_pywrap as p
from random import randint
from copy import copy

cardlist_ = ['7s', '8s', '9s', 'Ts', 'Js', 'Qs', 'Ks', 'As', '7c', '8c', '9c', 'Tc', 'Jc', 'Qc', 'Kc', 'Ac', '7d', '8d', '9d', 'Td', 'Jd', 'Qd', 'Kd', 'Ad', '7h', '8h', '9h', 'Th', 'Jh', 'Qh', 'Kh', 'Ah']

def readgames(filename):
	data = []
	labels = []
	states = []
	with open(filename) as f:
		gameIdx = 0
		while True:
		    gameIdx = gameIdx + 1
		    line = f.readline()
		    if not line:
		        break
		    p1 = p.CardsSet(line.split())
		    p2 = p.CardsSet(f.readline().split())
		    p3 = p.CardsSet(f.readline().split())
		    union = p.CardsSet([]).Add(p1).Add(p2).Add(p3)
		    if gameIdx % 200 == 0:
		        print gameIdx
		    first = int(f.readline())
		    gs_open = p.GameState([p1, p2, p3], first, 'n')
	            moves = f.readline().split()
        	    move_weights = [float(w) for w in f.readline().split()]
		    for hand in range(0, 3):
			gs = copy(gs_open)
			to_close = [0,1,2]
			to_close.remove(hand)
		        gs.CloseHands(to_close)
		        #moveidx = randint(0, 2)
			moveidx = 0
		        label = 0
			isgood = True
			for (move, weight) in zip(moves, move_weights):
		            if gs.GetMoveNumber() == moveidx and gs.GetCurPlayer() == hand:
		                label = p.GetCardIndex(move)
				#if label % 8 != 0 and cardlist_[label-1] in gs.Hand(hand):
				#	isgood = False
				#if label % 8 != 7 and cardlist_[label+1] in gs.Hand(hand):
				#	isgood = False
				if weight < 0.06:
					isgood = False
		                break
			    gs.MakeMove(move)
		        if isgood:
		            features = [a for (b,a) in p.CalcFeatures(gs, hand) if b == 'close_cards' or b == 'common_cards']
		            data.append(features)
        		    labels.append(label)
	       		    states.append((gs, gs_open, moves))
	return (data, labels, states)


def printhand(h):
        return ' '.join(h.ToArray())

def print_data(model, data, labels, states):
	for (probs, label, (gs1, gs2, moves)) in zip(model.predict(data)[0:20], labels[0:20], states[0:20]):
		print probs
		print label
		print printhand(gs1.Hand(0))
		print '******'
		print printhand(gs2.Hand(0))
		print printhand(gs2.Hand(1))
		print printhand(gs2.Hand(2))
		print '******'
		print ' '.join(moves)
		print '---------------------------------'


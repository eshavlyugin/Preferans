import Pref_pywrap as p
import pytest
from random import randint

class MyPlayer(p.PyPlayer):
    layout_ = None
    xray_ = None

    def on_new_layout(self, layout):
        self.layout_ = layout

    def on_new_xray_layout(self, layout):
        self.xray_ = xray

    def on_move(self, card):
	self.layout_.MakeMove(card)
        pass

    def do_move(self):
	valid = self.layout_.GenValidMoves()
        return valid[0]
        #return valid[randint(0, len(valid) - 1)]

class TestModule:
    def test_cards_set(self):
        h1 = p.CardsSet(['7h', '8s', 'Th'])
        assert '8s' in h1
        assert not '9d' in h1

    def test_game_state(self):
        h1 = p.CardsSet(['7h', '8s', 'Th'])
        h2 = p.CardsSet(['As', 'Ts', '8c'])
        h3 = p.CardsSet(['Ac', 'Ad', 'Qd'])

        game = p.GameState([h1, h2, h3], 0, 'n')

        assert not game.IsValidMove('7s')
        assert game.IsValidMove('7h')
        assert '7h' in game.Hand(0)
        assert not 'As' in game.Hand(0)
        assert not 'Qd' in game.Hand(0)

	print 'Features:', p.CalcFeatures(game, 1)[0:10]

        game.MakeMove('7h')
        assert not '7h' in game.Hand(0)

    def test_exception(self):
        h1 = p.CardsSet(['7h', '8s', 'Th'])
        h2 = p.CardsSet(['As', 'Ts', '8c'])
        h3 = p.CardsSet(['Ac', 'Ad', 'Qd'])
        with pytest.raises(RuntimeError):
            wrongState = p.GameState([h1, h2], 0, 'n')

        with pytest.raises(RuntimeError):
            h1.Add('dd')

        gs = p.GameState([h1, h2, h3], 0, 'n')
        with pytest.raises(RuntimeError):
            gs.MakeMove('8h')

    def test_player(self):
        player1 = MyPlayer()
        player2 = p.Player.create("random:random:random")
        player3 = p.Player.create("random:random:random")
        player4 = p.Player.create("random:random:random")
        manager = p.GameManager([player1, player2, player3])
        scores = [0, 0, 0]
	for i in range(0, 100000):
            manager.SetNewLayout(p.GenLayout(), False)
            manager.PlayToTheEnd()
            scores = [a + b for (a,b) in zip(scores, manager.GetState().GetScores())]
        print scores


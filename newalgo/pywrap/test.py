import Pref_pywrap as p

h1 = p.CardsSet(1 + 2 + 4)
h2 = p.CardsSet(8 + 16 + 32)
h3 = p.CardsSet(64 + 128 + 256)

game = p.GameState([h1, h2, h3], 0, 4)

print game.Hand(0).IsInSet(1)
print game.Hand(0).IsInSet(8)
print game.Hand(0).IsInSet(4)

print 'Features', p.CalcFeatures(game, 1)

game.MakeMove(1)
print game.Hand(0).IsInSet(1)

wrongGame = p.GameState([h1, h2], 0, 4)

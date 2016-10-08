import Pref_pywrap as p

scores = [0, 0, 0]

with open('game_rec.txt') as f:
  while True:
    line = f.readline()
    if not line:
      break
    p1 = p.CardsSet(line.split())
    p2 = p.CardsSet(f.readline().split())
    p3 = p.CardsSet(f.readline().split())
    first = int(f.readline())
    gs = p.GameState([p1, p2, p3], first, 'n')
    for move in f.readline().split():
      gs.MakeMove(move)
    scores = [a + b for (a,b) in zip(scores, gs.GetScores())]

print 'Final scores:', scores
print 'Total = ', sum(scores)

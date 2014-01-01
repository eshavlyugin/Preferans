import httplib;
import re;
import sys;

class PrefGameParser:
    moves = []
    hands = []
    trade = []
    widow = ""
    contract = ""
    firstPlayer = 0
    numOfPlayers = 0
    
    def __init__(self):
        self.Clear()
        
    def Clear(self):
        self.hands = []
        self.trade = []
        self.moves = []
        self.widow = ""
        self.firstPlayer = 0
        self.numOfPlayers = 0
        
    def Print(self):
        print "begin"
        print "CONTRACT " + self.contract
        print "FIRSTPLAYER " + str(self.firstPlayer)
        print "NUMOFPLAYERS " + str(self.numOfPlayers)
        for ss in self.hands:
            print "HAND " + ss
        for ss in self.trade:
            print "TRADE " + ss
        print "WIDOW " + self.widow
        for ss in self.moves:
            print "MOVE " + ss
        print "end"
    
    def ParseIndexToRealIndex(self, index):
        if index == 0:
            return index
        elif index == 1:
            return 3
        elif index == 3:
            return 2
        elif index == 2:
            return 1
        else:
            return 0
        
    def Parse(self, layouts, gameInfo, tradeInfo, sequence, playersInfo):
        self.numOfPlayers = len(playersInfo)
        self.contract = gameInfo[1]
        if self.contract == u"Распасы" or self.contract == u"пас":
            self.contract = "pass"
        elif self.contract == u"Мизер":
            self.contract = "miser"
        elif self.contract == u"RMZ":
            self.contract = "6-3"
        elif self.contract == '6-3':
            self.contract = '6-3'
        elif self.contract == '7-3':
            self.contract = '7-3'
        elif self.contract == '8-3':
            self.contract = '8-3'
        elif self.contract == '9-3':
            self.contract = '9-3'
        elif self.contract == '10-3':
            self.contract = '10-3'
        elif self.contract.find(u"БК") >= 0:
            self.contract = self.contract[0] + "nt"
        else:
            regexp = re.compile("/images/([a-z])\.gif")
            self.contract = self.contract[0] + regexp.search( self.contract ).group(1)
            
        if len(playersInfo) == 3:
            self.ParseLayoutsFor3Players(layouts)
            self.firstPlayer = self.ParseTrade3Players(tradeInfo)
            if self.contract == 'pass':
                self.ParseMovesSequence3(sequence)
        elif len(playersInfo) == 4:
            self.firstPlayer = self.ParseLayoutsFor4Players(layouts)
            self.ParseTrade3Players(tradeInfo)
            if self.contract == 'pass':
                self.ParseMovesSequence4(sequence, self.firstPlayer)
            if self.firstPlayer == 3:
                self.firstPlayer = 0
        else:
            raise("wrong number of players")
        
    def ParseMovesSequence3(self, sequence):
        index = 2
        while index < len(sequence):
            self.moves.append(sequence[index][1] + sequence[index][0])
            index = index + 1
    
    def ParseMovesSequence4(self, sequence, firstPlayer):
        index = 0
        while index < len(sequence):
            self.moves.append(sequence[index][1] + sequence[index][0])
            index = index + 1
    
    def IsValidLayout(self, layout):
        regexp = re.compile("([ &nbsp;AKQJ10987]+)")
        str = regexp.search(layout)
        return str != None and str.group(0) == layout
    
    def ParseTrade3Players(self, tradeInfo):
        index = 4
        firstPlayer = -1
        npass = 0
        while( index < len(tradeInfo) ):
            if( len(tradeInfo[index]) > 0 and firstPlayer == -1):
                firstPlayer = index - 4
            regexp = re.compile("/images/([a-z])\.gif")
            str = regexp.search(tradeInfo[index])
            if str != None:
                self.trade.append(tradeInfo[index][0] + str.group(1))
            elif tradeInfo[index].find(u"БК") >= 0:
                self.trade.append(tradeInfo[index][0] + "nt");
            elif tradeInfo[index] == u"пас":
                #pass vote
                self.trade.append("pass")
                npass = npass + 1
            elif len(tradeInfo[index]) > 4:
                #misere vote
                self.trade.append("miser")
            if npass >= 2 and len(self.trade) > 2:
                break
            index = index + 1
        return firstPlayer
    
    def ParseLayoutsFor3Players(self, layouts):
        index = 0
        while not self.IsValidLayout(layouts[index][1]):
            index = index + 1
        for i in range(0,4):
            player = ""
            x = 4
            if i == 2:
                x = 2
            for j in range(0,x):
                ar = layouts[index][1].split(' ')
                for str in ar:
                    if( str != '&nbsp' and len(str) > 0):
                        player = player + " " + str + layouts[index][0]
                index = index + 1
            if i == 2:
                self.widow = player
            else:
                self.hands.append(player)
            
    def ParseLayoutsFor4Players(self, layouts):
        index = 0
        while not self.IsValidLayout(layouts[index][1]):
            index = index + 1
        result = -1
        for i in range(0,4):
            player = ""
            x = index + 1
            while x - index < 4 and layouts[x][0] != u's':
                x = x + 1
            x = x - index
            for j in range(0,x):
                ar = layouts[index][1].split(' ')
                for str in ar:
                    if( str != '&nbsp' and len(str) > 0):
                        player = player + " " + str + layouts[index][0]
                index = index + 1
            if len(player) < 10:
                self.widow = player
                result = i
            self.hands.append(player)
        tmp = self.hands[2]
        self.hands[2] = self.hands[3]
        self.hands[3] = tmp
        tmp = self.hands[1]
        self.hands[1] = self.hands[3]
        self.hands[3] = tmp
        newHands = []
        for strng in self.hands:
            if len(strng) > 10:
                newHands.append(strng)
        self.hands = newHands
        return self.ParseIndexToRealIndex(result)
    
class PrefDBDownloader:
    # Constants Definitions
    # URL for game list
    Encoding = "utf-8"
    GameListUrl = "sgame?game=9&fromgameno="
    # URL for game
    GameUrl = "protocol?gameno="
    # number of games remains
    DBSize = 1
    # first game id
    FirstGame = ""
    # max deals in 1 bullet
    MaxDeals = 0
    
    # Variables definitions
    # connection to server
    conn = None
    # Number of remainig DB size
    GamesRemains = 0
    # game parser instance
    parser = None
    # file output
    fileOut = None
    # list of players
    PlayersList = None
    
    def __init__(self, dbSize, firstGame):
        self.conn = httplib.HTTPConnection("www.gambler.ru")
        self.GamesRemains = dbSize
        self.FirstGame = firstGame
        self.parser = PrefGameParser()
        self.PlayersList = None
        
    def ProcessSingleGame(self, gameId, playersInfo):
        #checking players ratings
       # for playerInfo in playersInfo:
        #    if playerInfo[0] not in self.PlayersList:
         #       return
        if self.GamesRemains > 0:
            self.conn.request("GET", "/php/" + self.GameUrl + gameId)
            r1 = self.conn.getresponse()
            data = unicode(r1.read(), self.Encoding)
            regexp = re.compile("protocol.php\?gameno=" + gameId + "&dealno=")
            result = regexp.search(data)
            
            if result is None:
                return
            for dealNo in range(1,100):
                if not self.ProcessSingleLayout(gameId, dealNo, playersInfo):
                    break
            
    def ProcessSingleLayout(self, gameId, layoutId, playersInfo):
        sys.stderr.write(str(gameId) + u" " + str(layoutId) + u"\n")
        try:
            if self.GamesRemains > 0:
                self.conn.request("GET", "/php/" + self.GameUrl + gameId + "&dealno=" + str(layoutId))
                r1 = self.conn.getresponse()
                data = r1.read()
                data = unicode(data, self.Encoding)
                regexp = re.compile("images/[a-z]\.gif");
                if regexp.search(data) == None:
                    return False
                self.GamesRemains = self.GamesRemains - 1
                self.ParseData(data, playersInfo)
                return True
        except:
            return False
        
    def GetPlayersNames(self, data):
        regexp = re.compile("href=/user/info\?uin=[0-9]+>([^<]+)<")
        return regexp.findall(data)
    
    def GetPlayersRatings(self, data):
        regexp = re.compile("<td>([-+0-9]+) \([^)]+\)</td>")
        return regexp.findall(data)
    
    def GetGamePlayersCounts(self, data):
        regexp = re.compile("<th rowspan=([0-9]+)>&nbsp;</th><td>")
        return regexp.findall(data)
    
    def GetGameSequence(self, data):
        regexp = re.compile("<a href=.*?/images/")
        return regexp.findall(data)
    
    def ParseData(self, data, playersInfo):
        regexp = re.compile("/images/([a-z])\.gif[^>]+>([^<]+)<")
        gameInfo = re.compile("Array\(n\(\"([^\"]+)\",\"([^\"]+)\",\"([^\"]+)\",\"([^\"]+)")
        trade = re.compile("<TD>(.*?)</TD>")
        sequence = re.compile("/images/([a-z])\.gif\' height=11 width=13 border=0.*?>([AKQJ10987]+?)<")
        self.PrintLayout(regexp.findall(data), gameInfo.search(data).groups(), trade.findall(data), sequence.findall(data), playersInfo)
    
    def GetPlayersList(self):
        i = 1
        lim = 50
        result = []
        while i < 1500:
            self.conn.request("GET", "/php/top.php?playtime=&game=9&max=2325&first=" + str(i) + "&lim=" + str(lim) + "&max=2325")
            r1 = self.conn.getresponse()
            sys.stderr.write("Status: " + str(r1.status))
            tmp = r1.read();
            data1 = unicode(tmp, self.Encoding)
            regexp = re.compile("class=notunderlined>(.*?)</a></td>")
            result = result + regexp.findall(data1)
            i = i + lim
            
        self.PlayersList = set(result)
        
    def PrintLayout(self, cards, gameInfo, trade, sequence, playersInfo):
        self.parser.Clear()
        self.parser.Parse(cards, gameInfo, trade, sequence, playersInfo)
        sys.stderr.write("Ok!!\n")
        self.parser.Print()
            
    def ProcessPage(self, firstGame):
        self.conn.request("GET", "/php/" + self.GameListUrl + firstGame)
        r1 = self.conn.getresponse()
        data1 = unicode(r1.read(), self.Encoding)
        playerNames = self.GetPlayersNames(data1)
        playersOnGames = self.GetGamePlayersCounts(data1)
        currentPlayer = 0
        players = []
        for cnt in playersOnGames:
            gamePlayers = []
            for i in range(0,int(cnt)):
                gamePlayers.append([playerNames[currentPlayer+i]])
            currentPlayer = currentPlayer + int(cnt)
            players.append(gamePlayers)
        regexp = re.compile("protocol\?gameno=([0-9]+)")
        result = regexp.findall(data1)
        index = 0
        for st in result:
            nPlayers = len(players[index])
            sys.stderr.write("Players: " + str(nPlayers))
            if len(players[index]) == 3:
                sys.stderr.write( 'Game parsed\n' + st )
                self.ProcessSingleGame(st, players[index])
            index = index + 1
        nextexp = re.compile("sgame\?game=9&fromgameno=([0-9]+)")
        result = nextexp.findall(data1)
        return result[1]
        
    def DoMainLoop(self):
        self.GetPlayersList()
        page = self.FirstGame;
        while self.GamesRemains > 0:
            sys.stderr.write(page + "\n")
            page = self.ProcessPage(page)
            
downloader = PrefDBDownloader(1500000, "80908969")
downloader.DoMainLoop();

#
# Random City Name Generator
# 

from CvPythonExtensions import *

import CvUtil
import sys
import math

gc = CyGlobalContext()	

# Returns a random city name
def getRandomCityName():
	constbeginlist = ["b","b","b","b","bh","bj","bl","br","c","c","c","c","ch","chr","cl","cr","cz","d","d","d","d","dh","dr","f","f","f","f","fh","fl","fr","g","g","g","g","gh","gl","gr","h","h","h","h","j","k","k","k","k","kl","kn","kr","l","l","l","l","m","m","m","m","n","n","n","n","p","p","p","p","ph","pl","pr","ps","q","r","r","r","r","rh","s","s","s","s","sc","scr","sh","shr","sk","skr","sl","sm","sn","sp","st","str","sv","sw","t","t","t","t","th","thr","tr","v","vh","vl","vr","w","wh","wr","x","z","zh","zr"]
	constbeginmildlist = ["b","bl","br","c","ch","chr","cl","cr","d","dr","f","fl","fr","g","gl","gr","h","j","k","kl","kr","l","m","n","p","pl","pr","r","s","sc","scr","sh","shr","sl","sm","sn","sp","st","str","sw","t","tr","v","w","wr","x","z"]
	constbegintypetwolist = ["b","b","b","b","bh","bj","c","c","c","c","ch","cz","d","d","d","d","dh","f","f","f","f","fh","g","g","g","g","gh","h","h","h","h","j","k","k","k","k","kn","k","l","l","l","l","m","m","m","m","n","n","n","n","p","p","p","p","ph","ps","q","s","s","s","s","sc","sh","sk","sm","sn","sp","st","sv","sw","t","t","t","t","th","v","vh","w","wh","x","z","zh"]
	constcaplist = ["B","B","Br","Br","Bj","Bl","Bl","C","C","Ch","Ch","Chr","Chr","Cl","Cl","Cr","Cr","Cz","D","D","Dr","Dr","F","F","Fr","Fr","Fl","Fl","G","G","Gh","Gh","Gr","Gr","H","H","J","J","K","K","Kh","Kl","Kl","Kn","Kn","Kr","Kr","L","L","M","M","N","N","P","P","Ph","Ph","Pr","Pr","Q","R","R","Rh","S","S","Sc","Sc","Scr","Scr","Sh","Sh","Shr","Shr","Sl","Sl","Sm","Sm","Sn","Sn","Sp","Sp","Spr","Spr","St","St","Str","Str","Sw","Sw","T","T","Th","Th","Thr","Thr","V","V","Vl","W","W","Wh","Wh","Wr","Wr","X","Z","Z","Zr"]
	constcaptypetwolist = ["B","B","Bj","C","C","Ch","Ch","Cz","D","D","F","F","G","G","Gh","Gh","H","H","J","J","K","K","Kh","Kn","Kn","L","L","M","M","N","N","P","P","Ph","Ph","Q","R","R","Rh","S","S","Sc","Sc","Sh","Sh","Sm","Sm","Sn","Sn","Sp","Sp","St","St","Sw","Sw","T","T","Th","Th","V","V","W","W","Wh","Wh","X","Z","Z"]
	constconnectlist = ["b","b","b","b","b","b","b","b","bb","bl","br","c","c","c","c","c","c","c","c","cc","ch","cl","cr","d","d","d","d","d","d","d","d","dd","dr","f","f","f","f","f","f","f","f","ff","fr","g","g","g","g","g","g","g","g","gg","gr","h","h","h","h","h","h","h","h","j","k","k","k","k","k","k","k","k","kk","kr","l","l","l","l","l","l","l","l","ll","ld","lh","m","m","m","m","m","m","m","m","mm","n","n","n","n","n","n","n","n","nn","nr","p","p","p","p","p","p","p","p","pp","pr","r","r","r","r","r","r","r","r","rr","rb","rc","rd","rf","rg","rk","rl","rm","rn","rp","rs","rt","rv","rz","s","s","s","s","s","s","s","s","ss","sh","sr","t","t","t","t","t","t","t","t","tt","th","tr","v","vr","w","x","z","zr"]
	constconnecttypetwolist = ["b","b","b","b","b","b","b","b","bb","c","c","c","c","c","c","c","c","cc","d","d","d","d","d","d","d","d","dd","f","f","f","f","f","f","f","f","ff","g","g","g","g","g","g","g","g","gg","h","h","h","h","h","h","h","h","j","k","k","k","k","k","k","k","k","kk","l","l","l","l","l","l","l","l","ll","m","m","m","m","m","m","m","m","mm","n","n","n","n","n","n","n","n","nn","nt","p","p","p","p","p","p","p","p","pp","r","r","r","r","r","r","r","r","rr","s","s","s","s","s","s","s","s","ss","sh","t","t","t","t","t","t","t","t","tt","th","v","w","x","z"]
	constendlist = ["b","b","b","b","c","c","c","c","ch","ck","d","d","d","d","f","g","g","g","g","gg","gh","ght","h","h","h","h","j","k","k","k","k","l","l","l","l","ll","ld","ln","m","m","m","m","n","n","n","n","nn","ng","ngth","ns","nt","p","p","p","p","ph","q","r","r","r","r","rr","rb","rc","rch","rd","rf","rg","rgh","rk","rl","rm","rn","rp","rs","rst","rsh","rt","rth","rv","rz","s","s","s","s","ss","sh","st","t","t","t","t","th","ts","tt","v","vl","w","x","z"]
	constendconnectlist = ["b","b","b","b","c","c","c","c","ck","d","d","d","d","f","g","g","g","g","h","h","h","h","k","k","k","k","l","l","l","l","ll","ld","ln","m","m","m","m","n","n","n","n","ng","ng","ng","ng","ns","p","p","p","p","q","r","r","r","r","rc","rf","rg","rk","rm","rn","rp","rs","rz","s","s","s","s","ss","t","t","t","t","v","w","x","z"]
	constendmildlist = ["b","c","ck","d","f","g","h","k","l","ld","m","n","ng","nt","p","ph","r","rb","rc","rd","rg","rk","rl","rm","rn","rp","rs","rt","rth","s","sh","st","t","th","v","w","x","z"]
	constendmildtypetwolist = ["b","c","ck","d","f","g","h","k","l","ld","m","n","ng","nt","p","ph","s","sh","st","t","th","v","w","x","z"]
	constendtypetwolist = ["b","b","b","b","c","c","c","c","ch","ck","d","d","d","d","f","f","f","f","g","g","g","g","gh","ght","h","h","h","h","j","k","k","k","k","l","l","l","l","ll","m","m","m","m","n","n","n","n","nn","ng","ngth","ns","nt","p","p","p","p","ph","q","r","r","r","r","s","s","s","s","ss","sh","st","t","t","t","t","th","ts","tt","v","w","x","z"]
	prefixlist = ["","","","","","","","","","","","","","","","","","","","","El ","Al ","Los ","Las ","St "]
	suffixlist = ["um","olis","ium","iem","us","an","a","ae","e","eo","i","ia","o","u","y","ay"]
	suffixtypetwolist = ["","","","","","","","","ville","burgh","burg","burg","ton","ton","town","borough","hill","mont","mont"]
	vowellist = ["a","a","a","a","a","a","a","a","a","a","a","a","a","a","a","a","e","e","e","e","e","e","e","e","e","e","e","e","e","e","e","e","i","i","i","i","i","i","i","i","i","i","i","i","i","i","i","i","o","o","o","o","o","o","o","o","o","o","o","o","o","o","o","o","u","u","u","u","u","u","u","u","u","y","ae","ai","au","ay","ea","ee","ei","eo","eu","ey","ia","ie","io","iu","oa","oe","oi","oo","ou","oy","ua","ue","uo","yo"]
	vowelbasiclist = ["a","e","i","o","u"]
	vowelcaplist = ["A","A","A","A","A","A","A","A","E","E","E","E","E","E","E","E","I","I","I","I","I","I","I","I","O","O","O","O","O","O","O","O","U","U","U","U","Y","Ai","Au","Ea","Ei","Eu","Ia","Ie","Io","Oa","Oi","Ou","Ya","Ye","Yi","Yo","Yu"]

	constbegin = constbeginlist[gc.getGame().getMapRand().get(len(constbeginlist), "Random Name")]
	constbeginmild = constbeginmildlist[gc.getGame().getMapRand().get(len(constbeginmildlist), "Random Name")]
	constbegintypetwo = constbegintypetwolist[gc.getGame().getMapRand().get(len(constbegintypetwolist), "Random Name")]
	constcap = constcaplist[gc.getGame().getMapRand().get(len(constcaplist), "Random Name")]
	constcaptypetwo = constcaptypetwolist[gc.getGame().getMapRand().get(len(constcaptypetwolist), "Random Name")]
	constconnect = constconnectlist[gc.getGame().getMapRand().get(len(constconnectlist), "Random Name")]
	constconnecttypetwo = constconnecttypetwolist[gc.getGame().getMapRand().get(len(constconnecttypetwolist), "Random Name")]
	constend = constendlist[gc.getGame().getMapRand().get(len(constendlist), "Random Name")]
	constendconnect = constendconnectlist[gc.getGame().getMapRand().get(len(constendconnectlist), "Random Name")]
	constendmild = constendmildlist[gc.getGame().getMapRand().get(len(constendmildlist), "Random Name")]
	constendmildtypetwo = constendmildtypetwolist[gc.getGame().getMapRand().get(len(constendmildtypetwolist), "Random Name")]
	constendtypetwo = constendtypetwolist[gc.getGame().getMapRand().get(len(constendtypetwolist), "Random Name")]
	prefix = prefixlist[gc.getGame().getMapRand().get(len(prefixlist), "Random Name")]
	suffix = suffixlist[gc.getGame().getMapRand().get(len(suffixlist), "Random Name")]
	suffixtypetwo = suffixtypetwolist[gc.getGame().getMapRand().get(len(suffixtypetwolist), "Random Name")]
	vowela = vowellist[gc.getGame().getMapRand().get(len(vowellist), "Random Name")]
	vowelb = vowellist[gc.getGame().getMapRand().get(len(vowellist), "Random Name")]
	vowelbasica = vowelbasiclist[gc.getGame().getMapRand().get(len(vowelbasiclist), "Random Name")]
	vowelbasicb = vowelbasiclist[gc.getGame().getMapRand().get(len(vowelbasiclist), "Random Name")]
	vowelbasicc = vowelbasiclist[gc.getGame().getMapRand().get(len(vowelbasiclist), "Random Name")]
	vowelcap = vowelcaplist[gc.getGame().getMapRand().get(len(vowelcaplist), "Random Name")]

	Nametypea = prefix + constcaptypetwo + vowelbasica + constconnect + vowelbasicb + constconnecttypetwo + vowelbasicc
	Nametypeb = constcap + vowelbasica + constconnecttypetwo + vowelbasicb + constconnect + vowelbasicc + constendtypetwo
	Nametypec = constcap + vowelbasica + constconnecttypetwo + vowelbasicb + constconnect + vowelbasicc + constendtypetwo + suffix
	Nametyped = constcap + vowelbasica + constconnecttypetwo + vowelbasicb + constconnect + suffix
	Nametypee = vowelcap + constend
	Nametypef = constcap + vowela + constendtypetwo
	Nametypeg = constcaptypetwo + vowela + constend
	Nametypeh = constcap + vowela + constendtypetwo + suffix
	Nametypei = constcaptypetwo + vowela + constendmild + constbeginmild + vowelb + constendtypetwo
	Nametypej = constcap + vowela + constendmildtypetwo + constbeginmild + vowelb + constendtypetwo
	Nametypek = constcap + vowela + constconnecttypetwo  + vowelb
	Nametypel = constcaptypetwo + vowela + constconnect + vowela
	Nametypem = vowelcap + constbegin + vowela + constendtypetwo
	Nametypen = vowelcap + constbegintypetwo + vowela + constend
	Nametypeo = vowelcap + constconnect + vowelbasica + constendtypetwo + suffix
	Nametypep = vowelcap + constconnect + vowelbasica + constconnecttypetwo + vowelbasica
	Nametypeq = constcap + vowelbasica + constconnecttypetwo + vowelbasicb + constendconnect + suffixtypetwo
	Nametyper = constcap + vowelbasica + constendconnect + suffixtypetwo
	Nametypes = vowelcap + constconnect + vowelbasica + constendconnect + suffixtypetwo
	Nametypet = prefix + constcap + vowela + constendtypetwo
	Nametypeu = prefix + vowelcap + constbegintypetwo + vowela + constend
	Nametypev = constcap + vowela + constendtypetwo + " " + constcaptypetwo + vowelb + constend
	Nametypew = constcap + vowela

	Mastername = [Nametypea,Nametypea,Nametypeb,Nametypeb,Nametypec,Nametypec,Nametyped,Nametyped,Nametypee,Nametypef,Nametypef,Nametypeg,Nametypeg,Nametypeh,Nametypeh,Nametypei,Nametypei,Nametypej,Nametypej,Nametypek,Nametypek,Nametypel,Nametypel,Nametypem,Nametypem,Nametypen,Nametypen,Nametypeo,Nametypeo,Nametypep,Nametypep,Nametypeq,Nametypeq,Nametyper,Nametyper,Nametypes,Nametypes,Nametypet,Nametypet,Nametypeu,Nametypeu,Nametypev,Nametypev,Nametypew]

	CityName = Mastername[gc.getGame().getMapRand().get(len(Mastername), "Random CityName")]

	return CityName
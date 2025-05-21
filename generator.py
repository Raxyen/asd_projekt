from random import randint, sample, seed
from math import ceil, sqrt, cos, sin, pi

#---config---
seed(2137)
n = 3 # liczba pol uprawnych
r = 15  # zageszczenie mapy || ilosc drog generowanych
l = ceil(sqrt(n * r**2))  # rozmiar mapy

#---limitacja losowan---
MIN_SPEC_POLE = 10
MAX_SPEC_POLE = 25

MIN_SPEC_BROWAR = 30
MAX_SPEC_BROWAR = 60

MIN_SPEC_KARCZMA = 20
MAX_SPEC_KARCZMA = 40

CROSS_CAPACITY_MIN = 30  # minimalna przepustowosc miedzy skrzyzowaniami
CROSS_CAPACITY_MAX = 100  # maksymalna przepustowosc miedzy skrzyzowaniami

CAPACITY_LIMIT = max(MAX_SPEC_BROWAR,MAX_SPEC_KARCZMA,MAX_SPEC_POLE) + 10  # domyslna przepustowosc krawedzi do skrzyzowan (placeholder do zapisu krawedzi w plik, musi byc > MAX_SPEC), trzeba bedzie zaimplementowac pomijanie maxow

EXTRA_EDGES_RANDOM = 2*n  # liczba dodatkowych losowych polaczen

ZNISZCZONE_DROGI_PERCENTAGE = 20 # jak nazwa sugeruje, szansa na to ze droga bedzie zniszczona
MIN_KOSZT_NAPRAWY_DROGI = 0
MAX_KOSZT_NAPRAWY_DROGI= 100

l = ceil(sqrt(n * r**2))  # rozmiar mapy
global_id = 0

# ---limitacja cwiartek ---
PODZIAL_X = 2
PODZIAL_Y = 2
WIELOKATY_LICZBA = PODZIAL_X * PODZIAL_Y
MIN_PUNKTOW =10
MAX_PUNKTOW = 30

MIN_BOOST_CWIARTKI = 110
MAX_BOOST_CWIARTKI = 200

#---pliki----
struktury = open("struktury.txt","w")
drogi = open("drogi.txt","w")


koordynat = []
lista_skrzyzowan = []
lista_nieskrzyzowan = []
krawedzie_miedzy_skrzyzowaniami = []

licznik_krawedzi_skrzyzowan = {}
zestaw_polaczen = set()

def create_rand(typ): #generowanie struktur 
    global global_id
    x,y = randint(0,l),randint(0,l)
    while([x,y]) in koordynat:
        x,y = randint(0,l),randint(0,l)
    koordynat.append([x,y])
    id = f"{global_id:03}{typ}"
    global_id+=1
    match(typ):
        case "1": #browary
            spec = randint(MIN_SPEC_BROWAR,MAX_SPEC_BROWAR)
            lista_nieskrzyzowan.append([id,x,y])
        case "2": #karczmy
            spec = randint(MIN_SPEC_KARCZMA,MAX_SPEC_KARCZMA)
            lista_nieskrzyzowan.append([id,x,y])
        case "3": #pola_uprawne
            spec = randint(MIN_SPEC_POLE,MAX_SPEC_POLE)
            lista_nieskrzyzowan.append([id,x,y])
        case "4": #skrzyzowania
            spec = 0
            lista_skrzyzowan.append([id,x,y])
    struktury.write(f'{x} {y} {id} {spec} \n')


def create_struct(n): #petle na generowanie struktur
    for i in range(n//2):create_rand("1") #browar
    for i in range(round(n*.8)):create_rand("2") #karczma
    for i in range(n):create_rand("3") #pola uprawne
    for i in range(2*n):create_rand("4") #skrzyzowania
    struktury.close()

def connect_to_crossroads(): #podlaczanie struktur do skrzyzowan
    for i in lista_nieskrzyzowan:
        struct_id,struct_x,struct_y = i
        najblizsze = min(lista_skrzyzowan,key = lambda x:(struct_x-x[1])**2+(struct_y-x[2])**2)
        skrzyz_id = najblizsze[0]
        przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
        drogi.write(f'{skrzyz_id} {struct_id} {CAPACITY_LIMIT} {0}\n')
        licznik_krawedzi_skrzyzowan[skrzyz_id]+=1

def connect_crossroads(): #MST + extra + filtr 
    krawedzie = []
    for i,skrzyzI in enumerate((lista_skrzyzowan)):
        for j, skrzyzJ in enumerate((lista_skrzyzowan[i+1:])):
            idI,xI,yI = skrzyzI
            idJ,xJ,yJ = skrzyzJ
            dystans = (xI-xJ)**2+(yI-yJ)**2
            krawedzie.append([dystans,idI,idJ])
    krawedzie.sort()
    parent = {}
    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    def union(x, y):
        root_x = find(x)
        root_y = find(y)
        if root_x != root_y:
            parent[root_y] = root_x
            return True
        return False

    for skrz in lista_skrzyzowan:
        parent[skrz[0]] = skrz[0]

    mst_krawedzie = []
    for d, a, b in krawedzie:
        if union(a, b):
            mst_krawedzie.append((a, b, d))

    for a, b, d in mst_krawedzie:
        przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
        krawedzie_miedzy_skrzyzowaniami.append((a, b, przepustowosc))
        licznik_krawedzi_skrzyzowan[a]+=1
        licznik_krawedzi_skrzyzowan[b]+=1
        zestaw_polaczen.add(tuple(sorted([a, b])))

    # Dodatkowe losowe połączenia 
    for _ in range(EXTRA_EDGES_RANDOM):
        a, b = sample(lista_skrzyzowan, 2)
        if a != b:
            id_a, id_b = a[0], b[0]
            a_id, b_id = sorted([id_a, id_b])
            if (a_id, b_id) not in zestaw_polaczen:
                przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
                zestaw_polaczen.add((a_id, b_id))
                krawedzie_miedzy_skrzyzowaniami.append((a_id, b_id, przepustowosc))
                licznik_krawedzi_skrzyzowan[a_id] += 1
                licznik_krawedzi_skrzyzowan[b_id] += 1

    #upewnianie sie ze nie ma pustych zaulkow
    for skrzyz_id in licznik_krawedzi_skrzyzowan:
        if licznik_krawedzi_skrzyzowan[skrzyz_id] == 1:
            kandydaci = [x[0] for x in lista_skrzyzowan if x[0] != skrzyz_id and tuple(sorted([skrzyz_id, x[0]])) not in zestaw_polaczen]
            if kandydaci:
                partner = sample(kandydaci, 1)[0]
                a_id, b_id = sorted([skrzyz_id, partner])
                zestaw_polaczen.add(tuple(sorted([a_id, b_id])))
                przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
                krawedzie_miedzy_skrzyzowaniami.append((a_id, b_id, przepustowosc))
                licznik_krawedzi_skrzyzowan[a_id] += 1
                licznik_krawedzi_skrzyzowan[b_id] += 1
                
    for a, b, p in krawedzie_miedzy_skrzyzowaniami:
        zniszczenie = 0
        if randint(0,100)<ZNISZCZONE_DROGI_PERCENTAGE:
            zniszczenie = randint(MIN_KOSZT_NAPRAWY_DROGI,MAX_KOSZT_NAPRAWY_DROGI)
        drogi.write(f'{a} {b} {p} {zniszczenie}\n')

def generate_wielokaty_wypukle():
    with open("cwiartki.txt", "w") as f:
        counter = 0
        for i in range(PODZIAL_Y):
            for j in range(PODZIAL_X):
                if counter >= WIELOKATY_LICZBA:
                    return

                xmin=j*(l//PODZIAL_X)
                xmax=(j+ 1)*(l//PODZIAL_X)
                ymin=i*(l//PODZIAL_Y)
                ymax=(i+1)*(l//PODZIAL_Y)

                cx=(xmin+xmax)//2
                cy=(ymin+ymax)//2
                polygon_r=min((xmax-xmin),(ymax-ymin))//2

                n=randint(MIN_PUNKTOW, MAX_PUNKTOW)
                angles=sorted([randint(0,360) for _ in range(n)])
                points=[
                    ((cx+polygon_r*cos(a*pi/180)),(cy + polygon_r*sin(a*pi/180)))
                    for a in angles
                ]
                boost = round(randint(110, 140)/100, 2)
                line = f"{boost} "+" ".join(f"{x} {y}" for x, y in points)
                f.write(line + "\n")
                counter += 1

create_struct(n)
licznik_krawedzi_skrzyzowan = {i[0]:0 for i in lista_skrzyzowan}
connect_to_crossroads()
connect_crossroads()
generate_wielokaty_wypukle()
drogi.close()

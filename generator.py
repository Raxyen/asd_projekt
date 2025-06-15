from random import randint, sample, seed
from math import ceil, sqrt, cos, sin, pi
global_id = 0
def generuj_graf(params):
    seed(params["SEED"])
    # ---config---
    MIN_SPEC_POLE = params["MIN_SPEC_POLE"]
    MAX_SPEC_POLE = params["MAX_SPEC_POLE"]

    MIN_SPEC_BROWAR = params["MIN_SPEC_BROWAR"]
    MAX_SPEC_BROWAR = params["MAX_SPEC_BROWAR"]

    MIN_SPEC_KARCZMA = params["MIN_SPEC_KARCZMA"]
    MAX_SPEC_KARCZMA = params["MAX_SPEC_KARCZMA"]

    CROSS_CAPACITY_MIN = params["CROSS_CAPACITY_MIN"]
    CROSS_CAPACITY_MAX = params["CROSS_CAPACITY_MAX"]
    EXTRA_EDGES_RANDOM = params["EXTRA_EDGES_RANDOM"]

    ZNISZCZONE_DROGI_PERCENTAGE = params["ZNISZCZONE_DROGI_PERCENTAGE"]
    MIN_KOSZT_NAPRAWY_DROGI = params["MIN_KOSZT_NAPRAWY_DROGI"]
    MAX_KOSZT_NAPRAWY_DROGI = params["MAX_KOSZT_NAPRAWY_DROGI"]

    LICZBA_BROWAROW = params["LICZBA_BROWAROW"]
    LICZBA_KARCZM = params["LICZBA_KARCZM"]
    LICZBA_POL = params["LICZBA_POL"]
    LICZBA_SKRZYZOWAN = params["LICZBA_SKRZYZOWAN"]

    # ---limitacja cwiartek ---

    PODZIAL_X = params["PODZIAL_X"]
    PODZIAL_Y = params["PODZIAL_Y"]
    WIELOKATY_LICZBA = PODZIAL_X * PODZIAL_Y
    MIN_BOOST_CWIARTKI = params["MIN_BOOST_CWIARTKI"]
    MAX_BOOST_CWIARTKI = params["MAX_BOOST_CWIARTKI"]
    # ---pliki----
    CAPACITY_LIMIT = max(MAX_SPEC_BROWAR, MAX_SPEC_KARCZMA, MAX_SPEC_POLE) + 10  # domyslna przepustowosc krawedzi do skrzyzowan [placeholder do zapisu krawedzi w plik, musi byc > MAX_SPEC], trzeba bedzie zaimplementowac pomijanie maxow
    r = 3*LICZBA_POL  # zageszczenie mapy || ilosc drog generowanych
    l = ceil(sqrt(LICZBA_POL * r ** 2))*5  # rozmiar mapy
    struktury = open("struktury.txt", "w")
    drogi = open("drogi.txt", "w")

    koordynat = []
    lista_skrzyzowan = []
    lista_nieskrzyzowan = []
    krawedzie_miedzy_skrzyzowaniami = []
    licznik_krawedzi_skrzyzowan = {}
    zestaw_polaczen = set()

    lista_skrzyzowan_miasto = []
    lista_nieskrzyzowan_miasto = []
    krawedzie_miedzy_skrzyzowaniami_miasto = []
    licznik_krawedzi_skrzyzowan_miasto = []
    zestaw_polaczen_miasto = set()

    cwiartki = [[[] for _ in range(PODZIAL_X)] for _ in range(PODZIAL_Y)]

    def create_rand(typ):  # generowanie struktur
        global global_id
        x, y = randint(0, l), randint(0, l)
        while ([x, y]) in koordynat:
            x, y = randint(0, l), randint(0, l)
        koordynat.append([x, y])
        cx, cy = min(PODZIAL_X - 1, x * PODZIAL_X // l), min(PODZIAL_Y - 1, y * PODZIAL_Y // l)
        cwiartki[cy][cx].append((x, y))
        id = f"{global_id:03}{typ}"
        global_id += 1
        match (typ):
            case "1":  # browary
                spec = randint(MIN_SPEC_BROWAR, MAX_SPEC_BROWAR)
                lista_nieskrzyzowan.append([id, x, y])
                lista_nieskrzyzowan_miasto.append([id, x, y])
            case "2":  # karczmy
                spec = randint(MIN_SPEC_KARCZMA, MAX_SPEC_KARCZMA)
                lista_nieskrzyzowan_miasto.append([id, x, y])
            case "3":  # pola_uprawne
                spec = randint(MIN_SPEC_POLE, MAX_SPEC_POLE)
                lista_nieskrzyzowan.append([id, x, y])
            case "4":  # skrzyzowania
                spec = 0
                lista_skrzyzowan.append([id, x, y])
            case "5":  # skrzyzowania
                spec = 0
                lista_skrzyzowan_miasto.append([id, x, y])
        struktury.write(f'{x} {y} {id} {spec} \n')


    def create_struct():  # petle na generowanie struktur
        for i in range(LICZBA_BROWAROW): create_rand("1")  # browar
        for i in range(LICZBA_KARCZM): create_rand("2")  # karczma
        for i in range(LICZBA_POL): create_rand("3")  # pola uprawne
        for i in range(LICZBA_SKRZYZOWAN): create_rand("4")  # skrzyzowania
        for i in range(LICZBA_SKRZYZOWAN): create_rand("5")  # skrzyzowania miasto
        struktury.close()

    def connect_to_crossroads(lista_skrzyz,lista_nieskrzyz,licznik_krawedzi):  # podlaczanie struktur do skrzyzowan
        for i in lista_nieskrzyz:
            struct_id, struct_x, struct_y = i
            najblizsze = min(lista_skrzyz, key=lambda x: (struct_x - x[1]) ** 2 + (struct_y - x[2]) ** 2)
            skrzyz_id = najblizsze[0]
            przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
            drogi.write(f'{skrzyz_id} {struct_id} {przepustowosc} {0}\n')
            licznik_krawedzi[skrzyz_id] += 1


    def connect_crossroads(lista_skrzyz,licznik_kraw,krawedzie_miedzy_skrzyz,zestaw_pol):  # MST + extra + filtr
        krawedzie = []
        for i, skrzyzI in enumerate((lista_skrzyz)):
            for j, skrzyzJ in enumerate((lista_skrzyz[i + 1:])):
                idI, xI, yI = skrzyzI
                idJ, xJ, yJ = skrzyzJ
                dystans = (xI - xJ) ** 2 + (yI - yJ) ** 2
                krawedzie.append([dystans, idI, idJ])
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
    
        for skrz in lista_skrzyz:
            parent[skrz[0]] = skrz[0]
    
        mst_krawedzie = []
        for d, a, b in krawedzie:
            if union(a, b):
                mst_krawedzie.append((a, b, d))
    
        for a, b, d in mst_krawedzie:
            przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
            krawedzie_miedzy_skrzyz.append((a, b, przepustowosc))
            licznik_kraw[a] += 1
            licznik_kraw[b] += 1
            zestaw_pol.add(tuple(sorted([a, b])))
    
        # Dodatkowe losowe połączenia
        for _ in range(EXTRA_EDGES_RANDOM):
            a, b = sample(lista_skrzyz, 2)
            if a != b:
                id_a, id_b = a[0], b[0]
                a_id, b_id = sorted([id_a, id_b])
                if (a_id, b_id) not in zestaw_pol:
                    przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
                    zestaw_pol.add((a_id, b_id))
                    krawedzie_miedzy_skrzyz.append((a_id, b_id, przepustowosc))
                    licznik_kraw[a_id] += 1
                    licznik_kraw[b_id] += 1
    
        # upewnianie sie ze nie ma pustych zaulkow
        for skrzyz_id in licznik_kraw:
            if licznik_kraw[skrzyz_id] == 1:
                kandydaci = [x[0] for x in lista_skrzyz if
                             x[0] != skrzyz_id and tuple(sorted([skrzyz_id, x[0]])) not in zestaw_pol]
                if kandydaci:
                    partner = sample(kandydaci, 1)[0]
                    a_id, b_id = sorted([skrzyz_id, partner])
                    zestaw_pol.add(tuple(sorted([a_id, b_id])))
                    przepustowosc = randint(CROSS_CAPACITY_MIN, CROSS_CAPACITY_MAX)
                    krawedzie_miedzy_skrzyz.append((a_id, b_id, przepustowosc))
                    licznik_kraw[a_id] += 1
                    licznik_kraw[b_id] += 1
    
        for a, b, p in krawedzie_miedzy_skrzyz:
            zniszczenie = 0
            if randint(0,100)<ZNISZCZONE_DROGI_PERCENTAGE:
                zniszczenie = randint(MIN_KOSZT_NAPRAWY_DROGI,MAX_KOSZT_NAPRAWY_DROGI)
            drogi.write(f'{a} {b} {p} {zniszczenie}\n')

    def convex_hull(points):
        points = sorted(set(points))
        if len(points) <= 1: return points
        def cross(o, a, b): return (a[0]-o[0])*(b[1]-o[1]) - (a[1]-o[1])*(b[0]-o[0])
        lower, upper = [], []
        for p in points:
            while len(lower) >= 2 and cross(lower[-2], lower[-1], p) <= 0: lower.pop()
            lower.append(p)
        for p in reversed(points):
            while len(upper) >= 2 and cross(upper[-2], upper[-1], p) <= 0: upper.pop()
            upper.append(p)
        return lower[:-1] + upper[:-1]

    def zapisz_cwiartki():
        with open("cwiartki.txt", "w") as f:
            for row in cwiartki:
                for punkty in row:
                    if len(punkty) >= 3:
                        otoczka = convex_hull(punkty)
                        boost = round(randint(MIN_BOOST_CWIARTKI, MAX_BOOST_CWIARTKI) / 100, 2)
                        f.write(f"{boost} " + " ".join(f"{x} {y}" for x, y in otoczka) + "\n")

    create_struct()
    licznik_krawedzi_skrzyzowan = {i[0]: 0 for i in lista_skrzyzowan}
    connect_to_crossroads(lista_skrzyzowan,lista_nieskrzyzowan,licznik_krawedzi_skrzyzowan)
    connect_crossroads(lista_skrzyzowan,licznik_krawedzi_skrzyzowan,krawedzie_miedzy_skrzyzowaniami,zestaw_polaczen)

    licznik_krawedzi_skrzyzowan_miasto = {i[0]: 0 for i in lista_skrzyzowan_miasto}
    connect_to_crossroads(lista_skrzyzowan_miasto,lista_nieskrzyzowan_miasto,licznik_krawedzi_skrzyzowan_miasto)
    connect_crossroads(lista_skrzyzowan_miasto,licznik_krawedzi_skrzyzowan_miasto,krawedzie_miedzy_skrzyzowaniami_miasto,zestaw_polaczen_miasto)

    zapisz_cwiartki()
    drogi.close()


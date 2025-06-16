import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.patches as patches

def rysuj_graf():
    # --- Wczytaj struktury ---
    koordynaty = {}
    specyfikacje = {}
    counter = {"1": 0, "2": 0, "3": 0, "4": 0, "5":0}

    with open("struktury.txt") as f:
        for linia in f:
            parts = linia.strip().split()
            x, y, id, spec = parts
            x, y = int(x), int(y)
            koordynaty[id] = (x, y)
            specyfikacje[id] = spec
            counter[id[-1]] += 1

    # --- Kolory dla typów ---
    kolory = {
        "1": "brown",   # browar
        "2": "orange",  # karczma
        "3": "green",   # pole
        "4": "gray",     # skrzyżowanie
        "5": "blue"    # skrzyzowanie_miasto
    }

    # --- Rysowanie struktur ---
    plt.figure(figsize=(10, 10))
    for id, (x, y) in koordynaty.items():
        typ = id[-1]
        plt.scatter(x, y, color=kolory.get(typ, "black"), s=60)
        if typ != "4":
            plt.text(x + 0.5, y + 0.5, f"{id}\nspec:{specyfikacje[id]}", fontsize=6)
        else:
            plt.text(x + 0.5, y + 0.5, id, fontsize=6)

    # --- Rysowanie dróg ---
    narysowane = set()
    with open("drogi.txt") as f:
        for linia in f:
            parts = linia.strip().split()
            if len(parts) < 3:
                continue
            a, b, p = parts[:3]
            if a in koordynaty and b in koordynaty:
                x1, y1 = koordynaty[a]
                x2, y2 = koordynaty[b]
                para = tuple(sorted([a, b]))
                if para not in narysowane:
                    plt.plot([x1, x2], [y1, y2], color="lightgray", linewidth=1)
                    if a[-1] == "4" and b[-1] == "4" or a[-1] == "5" and b[-1] == "5":
                        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
                        plt.text(mx, my, p, fontsize=6, color="blue")
                    narysowane.add(para)

    # --- Rysowanie ćwiartek z cwiartki.txt ---
    def wczytaj_cwiartki(filename="cwiartki.txt"):
        wielokaty = []
        with open(filename) as f:
            for linia in f:
                parts = linia.strip().split()
                boost = parts[0]
                coords = list(map(float, parts[1:]))
                punkty = [(coords[i], coords[i+1]) for i in range(0, len(coords), 2)]
                wielokaty.append((boost, punkty))
        return wielokaty

    wielokaty = wczytaj_cwiartki()

    for boost, punkty in wielokaty:
        polygon = patches.Polygon(punkty, closed=True, edgecolor="purple", fill=False, linewidth=1)
        plt.gca().add_patch(polygon)
        # podpisz boost na środku wielokąta
        sx = sum(x for x, _ in punkty) / len(punkty)
        sy = sum(y for _, y in punkty) / len(punkty)
        plt.text(sx, sy, f"x{boost}", color="purple", fontsize=8)

    # --- Legenda ---
    legend_elements = [
        mpatches.Patch(color='brown', label=f'Browar [{counter["1"]}]'),
        mpatches.Patch(color='orange', label=f'Karczma [{counter["2"]}]'),
        mpatches.Patch(color='green', label=f'Pole [{counter["3"]}]'),
        mpatches.Patch(color='gray', label=f'Skrzyżowanie [{counter["4"]}]'),
        mpatches.Patch(color='blue', label=f'Skrzyżowanie_Miasto [{counter["4"]}]'),
        mpatches.Patch(edgecolor='purple', facecolor='none', label='Ćwiartka (boost)', linewidth=1)
    ]
    plt.legend(handles=legend_elements, loc='upper right')

    plt.title("Wizualizacja miasta z ćwiartkami")
    plt.axis("equal")
    plt.grid(False)
    plt.show()

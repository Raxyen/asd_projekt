import tkinter as tk
from tkinter import messagebox
import threading
import os

from generator import generuj_graf
from wizualizator import rysuj_graf

CONFIG_FILE = "config.txt"

root = tk.Tk()
root.title("Generator grafu")
root.resizable(False, False)


range_entries = {}
entries = {}
tryby = {
    "LICZBA_BROWAROW": tk.StringVar(value="twarda"),
    "LICZBA_KARCZM": tk.StringVar(value="twarda"),
    "LICZBA_SKRZYZOWAN": tk.StringVar(value="twarda"),
}

row = 0


file_label = tk.Label(root, text="Nazwa pliku konfiguracyjnego:")
file_label.grid(row=row, column=0, sticky="w")
file_entry = tk.Entry(root, width=30)
file_entry.grid(row=row, column=1, columnspan=2, sticky="w")
file_entry.insert(0, CONFIG_FILE)
row += 1

def add_range(label, key_min, key_max):
    global row
    tk.Label(root, text=label).grid(row=row, column=0, sticky="w")
    e1 = tk.Entry(root, width=10)
    e1.grid(row=row, column=1)
    e2 = tk.Entry(root, width=10)
    e2.grid(row=row, column=2)
    range_entries[key_min] = e1
    range_entries[key_max] = e2
    row += 1


def add_number(label, key, tryb=False):
    global row
    tk.Label(root, text=label).grid(row=row, column=0, sticky="w")
    e = tk.Entry(root, width=10)
    e.grid(row=row, column=1)
    entries[key] = e
    if tryb:
        f = tk.Frame(root)
        f.grid(row=row, column=2, sticky="w")
        tk.Radiobutton(f, text="Ilość", variable=tryby[key], value="twarda").pack(side="left")
        tk.Radiobutton(f, text="Proporcja", variable=tryby[key], value="proporcja").pack(side="left")
    row += 1

# Dodaj pola
add_number("Seed (int)", "SEED")
add_number("Liczba pól", "LICZBA_POL")
add_number("Liczba browarów", "LICZBA_BROWAROW", tryb=True)
add_number("Liczba karczm", "LICZBA_KARCZM", tryb=True)
add_number("Liczba skrzyżowań", "LICZBA_SKRZYZOWAN", tryb=True)

add_range("Spec. pola (min/max)", "MIN_SPEC_POLE", "MAX_SPEC_POLE")
add_range("Spec. browaru (min/max)", "MIN_SPEC_BROWAR", "MAX_SPEC_BROWAR")
add_range("Spec. karczmy (min/max)", "MIN_SPEC_KARCZMA", "MAX_SPEC_KARCZMA")
add_range("Przepustowość skrzyż. (min/max)", "CROSS_CAPACITY_MIN", "CROSS_CAPACITY_MAX")
add_number("Dodatkowe losowe połączenia", "EXTRA_EDGES_RANDOM")
add_number("Procent zniszczonych dróg", "ZNISZCZONE_DROGI_PERCENTAGE")
add_range("Koszt naprawy drogi (min/max)", "MIN_KOSZT_NAPRAWY_DROGI", "MAX_KOSZT_NAPRAWY_DROGI")
add_number("Podział ćwiartek X", "PODZIAL_X")
add_number("Podział ćwiartek Y", "PODZIAL_Y")
add_range("Boost ćwiartki (min/max)", "MIN_BOOST_CWIARTKI", "MAX_BOOST_CWIARTKI")


def get_all_params():
    try:
        params = {}
        for key, e in entries.items():
            if key in tryby and tryby[key].get() == "proporcja":
                mnoznik = float(e.get())
                liczba_pol = int(entries["LICZBA_POL"].get())
                params[key] = round(mnoznik * liczba_pol)
            else:
                params[key] = int(e.get())
        for key, e in range_entries.items():
            params[key] = int(e.get())
        return params
    except ValueError as e:
        messagebox.showerror("Błąd", f"Nieprawidłowa wartość: {e}")
        return None


def wczytaj_config():
    config_name = file_entry.get().strip()
    if not config_name:
        messagebox.showwarning("Brak nazwy", "Podaj nazwę pliku konfiguracyjnego.")
        return
    if not os.path.exists(config_name):
        messagebox.showwarning("Brak pliku", f"{config_name} nie istnieje.")
        return
    with open(config_name, "r") as f:
        for line in f:
            if "=" not in line: continue
            key, val = line.strip().split("=", 1)
            if key in entries:
                entries[key].delete(0, tk.END)
                entries[key].insert(0, val)
                if key in tryby:
                    tryby[key].set("twarda")
            elif key.endswith("_MULT"):
                k = key.replace("_MULT", "")
                if k in entries:
                    entries[k].delete(0, tk.END)
                    entries[k].insert(0, val)
                    tryby[k].set("proporcja")
            elif key in range_entries:
                range_entries[key].delete(0, tk.END)
                range_entries[key].insert(0, val)
    messagebox.showinfo("OK", f"Wczytano dane z {config_name}")


def zapisz_config():
    config_name = file_entry.get().strip()
    if not config_name:
        messagebox.showwarning("Brak nazwy", "Podaj nazwę pliku konfiguracyjnego.")
        return
    try:
        with open(config_name, "w") as f:
            for key, e in entries.items():
                if key in tryby and tryby[key].get() == "proporcja":
                    f.write(f"{key}_MULT={e.get()}\n")
                else:
                    f.write(f"{key}={e.get()}\n")
            for key, e in range_entries.items():
                f.write(f"{key}={e.get()}\n")
        messagebox.showinfo("OK", f"Zapisano do {config_name}")
    except Exception as e:
        messagebox.showerror("Błąd zapisu", str(e))


def start_gen():
    params = get_all_params()
    if params:
        threading.Thread(target=lambda: generuj_graf(params), daemon=True).start()

def start_vis():
    threading.Thread(target=rysuj_graf, daemon=True).start()


tk.Button(root, text="Generuj graf", command=start_gen).grid(row=row, column=0, pady=10)
tk.Button(root, text="Rysuj graf", command=start_vis).grid(row=row, column=1, pady=10)
tk.Button(root, text="Wczytaj z pliku", command=wczytaj_config).grid(row=row+1, column=0, pady=5)
tk.Button(root, text="Zapisz do pliku", command=zapisz_config).grid(row=row+1, column=1, pady=5)

root.mainloop()

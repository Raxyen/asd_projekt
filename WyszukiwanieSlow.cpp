#include <iostream>
#include <string>
#include <fstream>
using namespace std;
class Wyszukiwanie {
public:
    string tekst;
    Wyszukiwanie(string nazwa);
    string algorytmNaiwny(string slowo) {
        string linia = "";
        string wynik = "";
        int liniaNum = 0;
        bool pasuje = false;
        for (int iter = 0; iter < tekst.length(); iter++) {
            linia += tekst[iter];
            if (tekst[iter] == '\n') {
                // linia wczytana
                liniaNum += 1;
                for (int litera = 0; litera < linia.length(); litera++) {
                    for (int j = 0; j < slowo.length(); j++) {
                        if (!(tolower(linia[litera + j]) == tolower(slowo[j]))) {
                            pasuje = false;
                            //cout << linia[litera + j] << " == " << slowo[j] << endl;
                            break;
                        }
                        else {
                            // cout << linia[litera + j] << " == " << slowo[j] << endl;
                            pasuje = true;
                        }
                    }
                    if (pasuje) {
                        wynik += "Linia " + to_string(liniaNum) + ": " + linia;
                        pasuje = false;
                        break;
                    }
                }
                linia = "";
            }
        }
        if (wynik.empty()) {
            return "Nie znaleziono\n";
        }
        else {
            return wynik;
        }
    }
    string algorytmBM(string slowo) {
        // alfabet uzyty {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z}
        int LAST[26];
        for (int pom = 0; pom < 25; pom++) {
            LAST[pom] = -1;
        }
        string linia = "";
        string wynik = "";
        int liniaNum = 0;
        for (int pom = 0; pom < slowo.length(); pom++) {
            if (((int)tolower(slowo[pom]) - (int)'a')<26 && ((int)tolower(slowo[pom]) - (int)'a')>=0) {
                LAST[(int)tolower(slowo[pom]) - (int)'a'] = pom;
            }
            //cout << slowo[pom] << LAST[(int)slowo[pom] - (int)'a'];
        }
        int i, j;
        for (int iter = 0; iter < tekst.length(); iter++) {
            linia += tekst[iter];
            if (tekst[iter] == '\n') {
                // linia wczytana
                liniaNum += 1;
                i = 0;
                int n = linia.length();
                int m = slowo.length();
                while (i <= n - m) {
                    j = m - 1;
                    while ((j >= 0) && (tolower(slowo[j]) == tolower(linia[i + j]))) {
                        j--;
                    }
                    if (j >= 0) {
                        //cout << linia << endl;
                        i += max(1, j - LAST[(int)tolower(linia[i + j]) - (int)'a']);
                        //cout << LAST[(int)linia[i + j] - (int)'a'] << endl;
                    }
                    else {
                        //cout << linia << " " << j;
                        wynik += "Linia " + to_string(liniaNum) + ": " + linia;
                        //i++;
                        break;
                        i++;
                    }
                }
                linia = "";
            }
        }
        if (wynik.empty()) {
            return "Nie znaleziono\n";
        }
        return wynik;
    }
    string algorytmKMP(string slowo) {
        string linia = "";
        string wynik = "";
        int j, i, liniaNum = 0;
        int m = slowo.length();
        int* Pi = new int[m];
        for (int pom = 0; pom < m; pom++) {
            Pi[pom] = 0;
        }
        int k = 0;
        for (int i = 2; i < m; i++) {
            while (k < 0 && Pi[k + 1] != Pi[i]) k = Pi[k];
            if (Pi[k + 1] == Pi[i]) k++;
            Pi[i] = k;
        }
        for (int iter = 0; iter < tekst.length(); iter++) {
            linia += tekst[iter];
            if (tekst[iter] == '\n') {
                // linia wczytana
                liniaNum += 1;
                int n = linia.length();
                j = 0;
                for (i = 0; i < n; i++) {
                    //cout << linia << endl << j << " " << slowo[j] << linia[i] << " " << Pi[j] << endl;
                    while ((j > 0) && (tolower(slowo[j]) != tolower(linia[i]))) {
                        j = Pi[j];
                        //cout << linia << endl << j << " " << slowo[j+1] << linia[i] << "" << Pi[j] << endl;
                    }
                    if (tolower(slowo[j]) == tolower(linia[i])) j++;
                    if (j == m) {
                        wynik += "Linia " + to_string(liniaNum) + ": " + linia;;
                        break;
                    }
                }
                linia = "";
            }
        }
        if (wynik.empty()) {
            return "Nie znaleziono\n";
        }
        return wynik;
    }
};
Wyszukiwanie::Wyszukiwanie(string nazwa) {
        string linia;
        ifstream dane(nazwa);
        while (getline(dane, linia)) tekst += linia + "\n";
}
int main() {
    string nazwaPliku, wzorzec;
    bool petla = true;
    int choice;
    cout << "Podaj plik docelowy: ";
    cin >> nazwaPliku;
    cout << "Podaj wzorzec : ";
    cin >> wzorzec;
    Wyszukiwanie dane = Wyszukiwanie(nazwaPliku);
    while (petla) {
        cout << "Podaj algorytm(1 - Naiwnny, 2 - Boyera Moora, 3 - Knutha-Morrisa-Pratta, 4 - zmiana wzorca, 5 - exit)" << endl;
        cin >> choice;
        switch (choice) {
        case 1:
            cout << "Naiwnny:\n" << dane.algorytmNaiwny(wzorzec) << endl;
            break;
        case 2:
            cout << "Boyera Moora:\n" << dane.algorytmBM(wzorzec) << endl;
            break;
        case 3:
            cout << "Knutha-Morrisa-Pratta:\n" << dane.algorytmKMP(wzorzec) << endl;
            break;
        case 4:
            cout << "Podaj wzorzec : ";
            cin >> wzorzec;
            break;
        case 5:
            petla = false;
            break;
        }
    }
    return 0;
}

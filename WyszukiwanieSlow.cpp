#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
string algorytmNaiwny(string dane, string slowo) {
    string linia="";
    string wynik = "";
    char * pomocniczaChar;
    bool pasuje = false;
    for (int iter = 0; iter < dane.length(); iter++) {
        linia += dane[iter];
        if (dane[iter] == '\n') {
            // linia wczytana
            for (int litera = 0; litera < linia.length(); litera++) {
                for (int j = 0; j < slowo.length(); j++) {
                    if (!(tolower(linia[litera+j]) == tolower(slowo[j]))) {
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
                    wynik += linia;
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
string algorytmBM(string dane, string slowo) {
    // alfabet uzyty {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z}
    int LAST[26];
    for (int pom = 0; pom < 26; pom++) {
        LAST[pom] = -1;
    }
    string linia = "";
    string wynik = "";
    for (int pom = 0; pom < slowo.length(); pom++) {
        LAST[(int)tolower(slowo[pom]) - (int)'a'] = pom;
        //cout << slowo[pom] << LAST[(int)slowo[pom] - (int)'a'];
    }
    int i, j;
    for (int iter = 0; iter < dane.length(); iter++) {
        linia += dane[iter];
        if (dane[iter] == '\n') {
        // linia wczytana
            i = 0;
            int n = linia.length();
            int m = slowo.length();
            while (i <= n-m) {
                j = m - 1;
                while ((j>=0) && (tolower(slowo[j])==tolower(linia[i+j]))) {
                    j--;
                }
                if (j > 0) {
                    //cout << linia << endl;
                    i += max(1, j - LAST[(int)tolower(linia[i+j]) - (int)'a']);
                    //cout << LAST[(int)linia[i + j] - (int)'a'] << endl;
                }
                else {
                    //cout << linia << " " << j;
                    wynik += linia;
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
/// <summary>
/// Algorytm KMP
/// </summary>
/// <param name="dane"></param>
/// <param name="slowo"></param>
/// <returns></returns>
string algorytmKMP(string dane, string slowo) {
    string linia = "";
    string wynik = "";
    int j, i;
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
    for (int iter = 0; iter < dane.length(); iter++) {
        linia += dane[iter];
        if (dane[iter] == '\n') {
            // linia wczytana
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
                    wynik += linia;
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

string wczytywanieDanych(string nazwa) {
    string tekst, linia;
    ifstream dane(nazwa);
    while (getline(dane, linia)) {
        tekst += linia+"\n";
    }
    return tekst;
}
int main()
{
    string dane, nazwaPliku, wzorzec;
    cin >> nazwaPliku;
    cin >> wzorzec;
    dane = wczytywanieDanych(nazwaPliku);
    //cout << dane << endl;
    cout << "Naiwnny:\n" << algorytmNaiwny(dane, wzorzec) << endl;
    cout << "Boyera Moora:\n" << algorytmBM(dane, wzorzec) << endl;
    cout << "Knutha-Morrisa-Pratta:\n" << algorytmKMP(dane, wzorzec) << endl;
    return 0;
}
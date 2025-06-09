# 🚗 Pojazd zbudowany z wykorzystaniem STM32

Projekt semestralny z przedmiotu **Systemy Wbudowane i Mikrokontrolery**  
Autorzy: Adrian Popielarczyk, Zuzanna Orzechowska   
Numery indeksu: 21295, 21284 
Data rozpoczęcia: 3 maja 2025
Repozytorium zawiera kod, dokumentację oraz materiały projektowe.

---

## 📌 Opis projektu

Robot potrafi poruszać się w dwóch trybach – automatycznym (porusza się po dowolnym czarnym śladzie na białym tle - linii) oraz manualnym (ruch sterowany jest za pomocą pilota oraz modułu podczerwieni). Robot posiada ekran LCD, potrafiący wyświetlać napisy oraz buzie – jej wyraz zmienia się wraz z kierunkiem jazdy w trybie automatycznym. Realizuje on te zadania za pomocą zewnętrznych przycisków. Robot posiada 1 parę osi. Oś nie jest skrętna, a przyczepność oraz napęd występują na tylnej osi. Skręt przypomina schemat poruszania się pojazdami pełzającymi jak czołg, z tą różnicą, że zamiast gąsienic mamy do czynienia z kołami. Robot posiada jedno źródło zasilania – do silników, oraz do płytki STM 32 – 6 baterii 1.6V. Podczas inicjalizacji programu, czyli uruchomieniu robota ogrywana jest skoczna melodyjka przez brzęczyk pasywny.

---

## 🛠️ Zastosowane technologie i narzędzia

- **Mikrokontroler:** STM32 NUCLEO F411RE
- **IDE:** STM32CubeIDE
- **Programowanie:** C (HAL / LL)
- **Sensory:**
  - Sensor podczerwieni IR 38 kHz
  - Sensory optyczne (IR) TCRT5000
- **Zasilanie:**  Bateria 1.6V x6
- **Sterownik silników:** L298n
- **Komunikacja:** UART (moduł podczerwieni)

---

## ⚙️ Funkcjonalności

- ✅ Napęd sterowany przez PWM z użyciem Timerów
- ✅ Obsługa sensorów ultradźwiękowych (pomiar odległości)
- ✅ Odczyt wartości z sensorów IR (linia / przeszkody) przy użyciu ADC
- ✅ Odgrywanie melodyjki podczas inicjacji programu
- ✅ Sterowanie ruchem przez UART (pilot z modułem podczerwieni)
- ✅ Zasilanie bateryjne – pełna autonomia
- ✅ Wyświetlacz LCD pokazujący wyraz twarzy - nos i oczy

---

## 📁 Struktura repozytorium
Trzy branch'e pokazujące postępy w tworzeniu projektu, wraz z nową zawartością kodu tworzono nowy realase - z paczką zip, obecnie najnowszy to release v3.1 - gotowa funkcjonalność jazdy manualnej i automatycznej, odgrywanie melodyjki oraz wyświetlacz LCD

---

## 🔌 Komendy UART

| Komenda         | Opis                               |
|-----------------|------------------------------------|
| `dzidaDoPrzodu` | Uruchamia pojazd - jazda do przodu |
| `stop`          | Zatrzymuje pojazd                  |
| `skretWLewo`    | Skręt w lewo                       |
| `skretWPrawo`   | Skręt w prawo                      |

---

## 🧪 Scenariusze testowe

- [x] Reakcja na białą/czarną linię (IR)
- [x] Komunikacja przez moduł podczerwieni
- [x] Test zasilania bateryjnego
- [x] Sterowanie ruchem w czasie rzeczywistym

---

## 📸 Demo i zdjęcia

- Zdjęcia pojazdu jak i nagrania znajdują się w folderze:
https://mega.nz/fm/jtxTxCZR

---

## 📄 Dokumentacja

Pełna dokumentacja projektu znajduje się w folderze:
https://mega.nz/fm/jtxTxCZR
w tym:
- Raport końcowy (PDF)
- Raporty do milestonów

---

## 📅 Harmonogram pracy

- 03.05 - 13.05: zaprojektowanie podstawowych sekwencji, budowa robota, raport z budowy,  projekt 3D robota, inżynieria
mechaniczna
- 14.05 - 27.05:  programowanie sekwencji dodatkowych, programowanie sekwencji podstawowej,raport z budowy 
- 28.05 - 03.06: dodanie dodatkowych funkcjonalności - brzęczyk pasywny i wyświetlacz LCD
- 10.06: finalna prezentacja robota

---

## 🧠 Wnioski

Robot bez zarzutu pokonuje trase po linii oraz może być sprawnie obsługiwany przez pilot z modułem podczerwieni. Dodatkowo sukcesem okazało sie dodanie wyświetlacza LCD, który ukazuje odpowiednią buzię wraz z kierunkiem jazdy robota w trybie automatycznym. Brzęczyk pasywny sygnalizuje, że cały program działa i robot odpowiednia inicjalizuje program i rozpoczyna jazde.

---

## 📬 Kontakt

W razie pytań:
- Email: 21295@student.ans-elblag.pl, 21284@student.ans-elblag.pl
- GitHub: krlpopiel, zuzanna-orzechowska

---

**Licencja:** MIT  

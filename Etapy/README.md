# Laboratorium 3 — wpisywanie rozwiązania krok po kroku

Każdy folder zawiera pełną kopię `Src` dla danego etapu. Etapy są narastające: folder 02 zawiera etap 01 oraz kolejne zmiany, folder 03 zawiera etapy 01–02 itd. Nie należy dodawać wszystkich tych folderów do kompilacji jednocześnie. Oryginalny folder projektu `Src` pozostał punktem wyjścia.

## Jak korzystać z etapów

1. Otwórz projekt w STM32CubeIDE. Przed zastępowaniem plików zachowaj kopię własnych zmian.
2. Otwórz pliki z `01_USART/Src` i porównaj je z plikami projektu w `Src`. Przepisz zmiany albo skopiuj zawartość plików etapu do plików o tych samych nazwach w głównym `Src`.
3. Zbuduj projekt. Powtórz tę procedurę kolejno dla folderów 02, 03, 04 i 05. Aby uruchomić tylko rozwiązanie końcowe, wystarczy skopiować pliki z `05_Petla_4_20mA/Src` do głównego `Src`.
4. W plikach od etapu 03 ustaw `STATION_ADDRESS` na rzeczywisty adres z obudowy stanowiska. **Wartość 1 w przykładzie jest tylko przykładem**, nie została odczytana ze stanowiska.
5. W CubeIDE wykonaj Clean i Build. Dostarczone w projekcie pliki `Debug` zawierają ścieżki do komputera poprzedniego użytkownika; CubeIDE powinno wygenerować je ponownie po imporcie projektu.

Foldery etapów używają wspólnych katalogów projektu `Inc`, `Drivers`, `Libraries` i `Startup`. Wspólny plik `Libraries/modbus/modbus.c` został zmieniony: zmienna `state` ma `volatile`, ponieważ jej wartość jest zmieniana w przerwaniu, a sprawdzana także w kodzie głównym. Ta poprawka dotyczy wszystkich etapów i powinna zostać zachowana.

## 01 — konfiguracja USART

W `main.c` uzupełnij kolejno:

- `RCC_Config()` — taktowanie USART1;
- `GPIO_Config()` — PA9: funkcja alternatywna push-pull, PA10: wejście pływające;
- `USART1_Config()` — 115200, osiem bitów danych, parzystość even, jeden bit stopu, nadawanie i odbiór.

To odpowiedniki listingów ze skryptu, z prędkością 115200 wymaganą w treści ćwiczenia. W STM32 ustawienie 9-bitowego słowa z parzystością oznacza tutaj 8 bitów danych i bit parzystości. Pozostałe `TODO` dotyczą następnych etapów; ten etap nie realizuje jeszcze komunikacji Modbus.

## 02 — timer i obsługa Modbus

W `main.c` uzupełnij konfigurację TIM4, przerwania NVIC i funkcje `Communication_*`, `Enable50usTimer()` oraz `Disable50usTimer()`. Wywołaj `MB_Config(115200)`. Przy zegarze timera 72 MHz preskaler 72 i okres 50 dają przerwanie co 50 µs.

W `stm32f10x_it.c` dopisz `Timer50usTick()` w obsłudze TIM4 oraz wywołania funkcji biblioteki w obsłudze USART1. Zgodnie ze skryptem SysTick wywołuje `MB()`, `TimeoutTick()` i `DelayTick()` co 10 µs. W `delay.c` licznik opóźnienia ma `volatile`, bo jest zmieniany w przerwaniu.

Obsługa USART korzysta z funkcji pokazanych w skrypcie, ale po RXNE tymczasowo wyłącza to przerwanie: zapobiega to jego wielokrotnemu wyzwalaniu przed odczytem bajtu przez `MB()`. `Communication_Get()` odczytuje bajt i ponownie włącza RXNE. Ten etap przygotowuje komunikację, ale sam nie wysyła jeszcze żądań.

## 03 — regulator

W `main.c` dodano `read_t1()`, `write_output()` i `regulate()` oraz ich wywołania w pętli głównej. Biblioteka dołącza adres i CRC; tablice żądań zawierają dane funkcji Modbus:

| Czynność | Funkcja | Rejestr |
| --- | --- | --- |
| Odczyt temperatury T1 | 04 — Input Register | 0 |
| Zapis mocy W1 | 06 — Holding Register | 0 |
| Zapis mocy G1 | 06 — Holding Register | 4 |

Temperatura T1 jest w setnych stopnia. Przykład przyjmuje zadaną 25,00°C (`SETPOINT_CENTI_C = 2500`), strefę ±1,00°C i moc 200 promili, czyli 20%. Poniżej strefy pracuje grzałka, powyżej wentylator, a w strefie oba są wyłączone. Przed włączeniem elementu kod wysyła polecenie wyłączenia drugiego.

Odpowiedzi są sprawdzane, w tym echo zapisu. LED1 sygnalizuje błąd, a program próbuje wysłać polecenia wyłączenia. **Przy zerwanej komunikacji nie można potwierdzić stanu wyjść**, dlatego zachowanie stanowiska w takim przypadku trzeba sprawdzić z prowadzącym. Pomiary temperatury w kodzie są interpretowane jako 16-bitowe wartości ze znakiem; sposób reprezentacji wartości ujemnych należy potwierdzić w dokumentacji stanowiska.

## 04 — LCD

`LCD_Refresh()` pokazuje w pierwszym wierszu temperaturę stanowiska `T` oraz zadaną `Z`, z dokładnością do 0,1°C. `T:ERR` sygnalizuje błąd odczytu lub sterowania. Wartość zadana jest stałą w kodzie; skrypt nie wymaga sposobu jej zmiany z klawiatury. Przy zmianie stałej należy stosować wartości z zakresu 0–99,99°C, zgodnie z formatem wyświetlania przykładu.

## 05 — pomiar z pętli prądowej

W `GPIO_Config()` ustaw PB0 jako wejście analogowe. W `ADC1_2_IRQHandler()` zapisz wynik konwersji do `adc_raw` i ustaw `adc_ready`. Istniejący TIM2 okresowo wyzwala ADC; wynik w `LCD_Refresh()` jest przeliczany na temperaturę i wyświetlany jako `P` w drugim wierszu.

Przeliczenie opiera się na rezystorze 160 Ω, odniesieniu 3,3 V i zakresie −30…60°C ze skryptu. Wskazanie poniżej 0,64 V jest oznaczane `P:BLAD <4mA`; w pobliżu tej granicy rzeczywiste tolerancje pomiaru mogą wpływać na wynik. Gwiazdka zastępuje znak stopnia.

## Sprawdzenie na stanowisku

1. Sprawdź z prowadzącym połączenia zasilania 24 V, rezystora 160 Ω, masy i PB0 zgodnie ze schematem ze skryptu. Nie zamieniaj masy i wejścia ADC.
2. Ustaw prawidłowy adres Modbus, sprawdź konwerter RS-485 i parametry 115200 8E1.
3. Sprawdź odczyt T1 i odpowiedzi Modbus, następnie osobno wyłączenie oraz włączenie W1 i G1 przy ustalonej mocy.
4. Sprawdź zachowanie poniżej i powyżej wartości zadanej oraz w strefie ±1°C. Stanowisko odświeża wartości co 1 s.
5. Porównaj odczyt z pętli z temperaturą referencyjną. Sprawdź sygnalizację błędu komunikacji i zachowanie po jej przywróceniu.

Wszystkie pięć etapów zostało skompilowanych i zlinkowanych kompilatorem ARM z zainstalowanego STM32CubeIDE. Nie były testowane na fizycznym stanowisku.

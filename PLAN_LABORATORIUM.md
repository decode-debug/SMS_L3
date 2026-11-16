# Plan rozwiązania laboratorium 3

## Cel

Zbudować regulator temperatury stanowiska grzejąco-chłodzącego komunikujący się przez Modbus RTU oraz niezależnie mierzyć temperaturę czujnikiem z pętlą prądową 4–20 mA. Wyświetlić temperaturę stanowiska, wartość zadaną i wynik pomiaru z pętli na LCD.

## 1. Przygotowanie komunikacji

- Skonfigurować USART1: PA9 jako TX i PA10 jako RX.
- Ustawić transmisję na **115200 bodów, 8E1** i wyłączyć sprzętową kontrolę przepływu. Skrypt pokazuje też przykład 19200 bodów, ale dla stanowiska wymaga 115200.
- Odczytać adres Modbus z oznaczenia stanowiska.
- Włączyć taktowanie USART1 oraz skonfigurować jego przerwania w NVIC.

## 2. Uruchomienie biblioteki Modbus

- Skonfigurować TIM4 do odmierzania kwantów 50 µs i włączyć obsługę jego przerwania.
- Uzupełnić `Communication_Mode()`, `Communication_Put()`, `Communication_Get()`, `Enable50usTimer()` i `Disable50usTimer()` w `Src/main.c`.
- W `Src/stm32f10x_it.c` podłączyć zdarzenia USART1 do biblioteki oraz wywołać `Timer50usTick()` w przerwaniu TIM4.
- Wywołać `MB_Config(115200)` i sprawdzić, czy SysTick co 10 µs obsługuje `MB()`, licznik limitu czasu oraz opóźnienia.
- Ustalić priorytety przerwań tak, aby komunikacja i odmierzanie czasu działały poprawnie.

## 3. Wymiana danych ze stanowiskiem

Wysyłać jedno żądanie i oczekiwać na odpowiedź przed wysłaniem następnego. Wykorzystać następujące rejestry:

| Element | Rodzaj rejestru | Adres | Jednostka |
| --- | --- | ---: | --- |
| Temperatura T1 | Input Register | 0 | 0,01°C |
| Wentylator W1 | Holding Register | 0 | promile pełnej mocy |
| Grzałka G1 | Holding Register | 4 | promile pełnej mocy |

- Przygotować żądania odczytu T1 i zapisu mocy W1 oraz G1.
- Sprawdzać stan, długość i zawartość odpowiedzi; obsłużyć timeout, błędny adres lub funkcję oraz odpowiedź wyjątkową.
- Uwzględnić, że stanowisko odświeża pomiary i sterowanie z częstotliwością 1 Hz.

## 4. Regulator temperatury

- Poniżej dolnej granicy wokół temperatury zadanej włączyć G1 i wyłączyć W1.
- Powyżej górnej granicy włączyć W1 i wyłączyć G1.
- W strefie wokół temperatury zadanej wyłączyć oba elementy.
- Jako punkt wyjścia przyjąć strefę nieczułości ±1°C; moc i szerokość strefy dobrać podczas prób.
- Zaplanować kolejność zapisów tak, aby podczas przejścia między stanami grzałka i wentylator nie pracowały jednocześnie.
- Nie traktować starego pomiaru jako aktualnego po błędzie komunikacji; zweryfikować reakcję na błąd na stanowisku.

## 5. Pomiar temperatury z pętli 4–20 mA

- Skonfigurować PB0 jako wejście analogowe ADC1 i regularnie zapisywać wynik konwersji.
- Przeliczać wynik ADC na temperaturę. Dla rezystora 160 Ω, zakresu −30…60°C i napięcia odniesienia 3,3 V:

  `T = ((ADC × 3,3 / 4095) − 0,64) × 90 / 2,56 − 30`

- Sprawdzić punkty kontrolne: około 0,64 V odpowiada −30°C, a 3,20 V odpowiada 60°C.
- Sygnalizować odczyt poniżej zakresu 4 mA jako możliwy problem z czujnikiem lub połączeniem.

## 6. LCD i wartość zadana

- Wyświetlić jednocześnie temperaturę T1, temperaturę zadaną i temperaturę z pętli prądowej. Znak stopnia można zastąpić `*`.
- Skrypt nie określa sposobu zmiany wartości zadanej: wykorzystać dostępną klawiaturę i opisać przypisanie klawiszy.
- Dodać czytelną sygnalizację błędów pomiaru i komunikacji.

## 7. Testy i zaliczenie

1. Skompilować projekt i usunąć błędy kompilacji.
2. Sprawdzić osobno LCD, klawiaturę i pomiar ADC.
3. Sprawdzić pojedyncze odczyty i zapisy Modbus, w tym odpowiedź na brak komunikacji.
4. Na stanowisku sprawdzić grzanie, strefę nieczułości, chłodzenie i zmianę wartości zadanej.
5. Sprawdzić zachowanie po utracie komunikacji i po jej przywróceniu.
6. Zanotować ustawienia i wyniki potrzebne do przedstawienia prowadzącemu. Skrypt nie podaje odrębnych wymagań dotyczących sprawozdania.

Źródło wymagań: `SMS_skrypt_25Z.pdf`, rozdział 3, w szczególności punkt 3.3.

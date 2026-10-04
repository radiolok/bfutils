# Brainfuck-100

Сто реальных программ на Brainfuck с GitHub, по которым выбиралась разрядность счётчиков [DekatronPC](https://github.com/radiolok/dekatronpc). Для каждой программы снято, сколько она занимает памяти программ, сколько ячеек ленты трогает, насколько глубоко уходит счётчик вложенности при промотке цикла и сколько инструкций исполняет.

*A set of 100 real-world Brainfuck programs from GitHub, sorted by category, with inputs, per-program metrics (program length, tape cells used, loop nesting reached during bracket scans, instructions executed) and source links. Used to size the DekatronPC counters.*

## Зачем

В DekatronPC каждый разряд счётчика — отдельный декатрон со своей логикой, поэтому лишний разряд стоит ламп, а недостающий — программ, которые машина не сможет выполнить. Вместо оценки «на глаз» разрядность проверена на настоящих программах. Итог — конфигурация **5 + 2 + 5 + 3 декатрона** (IP, Loop, AP, Data), в неё помещаются 97 программ из 100 (TRS DekatronPC, раздел 4.4, требования REQ-ARCH-010/011). Любое изменение разрядности проверяется на этом наборе заново.

| Счётчик | Декатронов | Диапазон | Что ограничивает | Не помещается |
|---|---:|---|---|---|
| IP | 5 | 0…99 999 (99 900 команд, верх — ПЗУ загрузчика) | длину программы | `LostKng.b` — 2,1 млн команд |
| Loop | 2 | 0…99 | вложенность при промотке | — (максимум в наборе 67, `Kiloseconds.b`) |
| AP | 5 | 0…29 999 | занятую ленту | `awib-0.4.b` на длинном входе, бесконечный `random.bf` |
| Data | 3 | 0…255 по кругу | значение ячейки | — |

Помещаются: **97 из 100**.

## Как сняты метрики

- Интерпретатор на C с 8-битными ячейками и переходом через 0 и 255; лента в обе стороны, поэтому видно и уход левее нулевой ячейки (на DekatronPC это переход 0 → 29 999).
- **Команды** — число команд Brainfuck после удаления комментариев и свёртки `[-]` в одну команду: столько ячеек займёт программа в памяти программ DekatronPC.
- **Ячеек** — сколько ячеек ленты программа тронула; **AP** — крайние значения указателя.
- **Loop** — максимум счётчика вложенности при промотке: счётчик считает от скобки, с которой началась промотка, поэтому он почти всегда меньше статической глубины вложенности в тексте.
- **Инструкций** — исполненных команд; **промотка** — шагов IP, пройденных при пропуске и повторе тела цикла. В среднем на исполненную инструкцию приходится 1,7 шага промотки.
- Программы, читающие ввод, запускались на файле `*.in` рядом с программой. Конец ввода — ноль, если в `metrics.json` не указано иное (`eof_mode`).
- Бесконечные генераторы остановлены на 5·10⁹ инструкций; их метрики — на момент остановки.

## Состав

| Папка | Категория | Программ |
|---|---|---:|
| [`demo/`](#демо) | Демо | 19 |
| [`math/`](#математика) | Математика | 30 |
| [`text/`](#текст) | Текст | 17 |
| [`algorithms/`](#алгоритмы) | Алгоритмы | 10 |
| [`interpreters/`](#интерпретаторы-и-компиляторы) | Интерпретаторы и компиляторы | 11 |
| [`quines/`](#квайны) | Квайны | 6 |
| [`games/`](#игры-и-графика) | Игры и графика | 4 |
| [`benchmarks/`](#нагрузочные-тесты) | Нагрузочные тесты | 3 |

Рядом с программами — `metrics.json`: все метрики, источник и лицензия каждой программы в машиночитаемом виде.

### Демо

Hello World, песни и картинки в ASCII, FizzBuzz: короткие программы, на которых машину показывают людям.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 1 | [`Hello.b`](demo/001_Hello.b) | Hello World из Википедии | 138 | 7 | 2 | 813 | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Hello.b) |
| 2 | [`hellbox-103.b`](demo/002_hellbox-103.b) | Hello World в 103 команды, Robert de Bath; вход: [`.in`](demo/002_hellbox-103.b.in) | 326 | 8 | 3 | 637 | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/extras/hellbox-103.b) |
| 3 | [`hello_world.bf`](demo/003_hello_world.bf) | Hello World, короткий вариант | 111 | 5 | 1 | 390 | [Wilfred/bfc](https://github.com/Wilfred/bfc/blob/master/sample_programs/hello_world.bf) |
| 4 | [`jabh.bf`](demo/004_jabh.bf) | «Just another brainfuck hacker», Д. Кристофани | 176 | 9 | 2 | 1 325 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/jabh.bf) |
| 5 | [`Beer.b`](demo/005_Beer.b) | «99 бутылок пива» целиком | 1 648 | 13 | 4 | 1,7·10⁶ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Beer.b) |
| 6 | [`bottles-3.bf`](demo/006_bottles-3.bf) | «99 бутылок», другая реализация | 3 032 | 100 | 4 | 2,9·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/bottles-3.bf) |
| 7 | [`squares.bf`](demo/007_squares.bf) | квадраты 0…10000, Д. Кристофани | 195 | 26 | 5 | 2,2·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/squares.bf) |
| 8 | [`sierpinski.bf`](demo/008_sierpinski.bf) | треугольник Серпинского в ASCII | 207 | 107 | 4 | 1,2·10⁵ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/sierpinski.bf) |
| 9 | [`sierpinski_carpet.bf`](demo/009_sierpinski_carpet.bf) | ковёр Серпинского; вход: [`.in`](demo/009_sierpinski_carpet.bf.in) | 833 | 15 | 7 | 7,6·10⁶ | [4ffy/brainfuck-programs](https://github.com/4ffy/brainfuck-programs/blob/main/sierpinski_carpet.bf) |
| 10 | [`xmastree.bf`](demo/010_xmastree.bf) | ёлочка заданной высоты; вход: [`.in`](demo/010_xmastree.bf.in) | 133 | 48 | 3 | 1,5·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/asciiart/xmastree.bf) |
| 11 | [`asciiart.bf`](demo/011_asciiart.bf) | ASCII-арт, печатает программу на C; вход: [`.in`](demo/011_asciiart.bf.in) | 3 072 | 44 | 3 | 3,8·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/asciiart/asciiart.bf) |
| 12 | [`write-language-name-in-3d-ascii.bf`](demo/012_write-language-name-in-3d-ascii.bf) | название языка объёмными буквами | 992 | 8 | 2 | 2 064 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Write-language-name-in-3D-ASCII/Brainf---/write-language-name-in-3d-ascii.bf) |
| 13 | [`chess.b`](demo/013_chess.b) | шахматная доска с фигурами | 5 053 | 9 | 1 | 5 697 | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/chess.b) |
| 14 | [`fizzbuzz.bf`](demo/014_fizzbuzz.bf) | FizzBuzz 1…100 | 727 | 16 | 2 | 2 838 | [Wilfred/bfc](https://github.com/Wilfred/bfc/blob/master/sample_programs/fizzbuzz.bf) |
| 15 | [`bizzfuzz.bf`](demo/015_bizzfuzz.bf) | FizzBuzz с вычислением, а не таблицей | 692 | 39 | 5 | 1,7·10⁵ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/bizzfuzz.bf) |
| 16 | [`show-ascii-table.bf`](demo/016_show-ascii-table.bf) | таблица ASCII | 99 | 4 | 2 | 954 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Show-ASCII-table/Brainf---/show-ascii-table.bf) |
| 17 | [`terminal-control-ringing-the-terminal-bell.bf`](demo/017_terminal-control-ringing-the-terminal-bell.bf) | звонок терминалу (BEL) | 12 | 1 | 0 | 12 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Terminal-control-Ringing-the-terminal-bell/Brainf---/terminal-control-ringing-the-terminal-bell.bf) |
| 18 | [`order.bf`](demo/018_order.bf) | меню-диалог «заказ в фастфуде»; вход: [`.in`](demo/018_order.bf.in) | 20 412 | 24 | 3 | 2,1·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/order.bf) |
| 19 | [`oobrain.bf`](demo/019_oobrain.bf) | объектно-ориентированная программа: фигуры и методы | 17 744 | 208 | 7 | 5,8·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/oobrain.bf) |

### Математика

π, e, простые числа, факторизация, Коллатц, Фибоначчи, калькуляторы. Здесь самые долгие вычисления и несколько бесконечных генераторов.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 20 | [`pi.bfk`](math/020_pi.bfk) | π, программа из проекта DekatronPC | 618 | 66 | 11 | 2,0·10⁵ | [DekatronPC](https://github.com/radiolok/dekatronpc/blob/master/rtl/programs/pi.bfk) |
| 21 | [`pi-digits.bf`](math/021_pi-digits.bf) | π до заданного знака (30); вход: [`.in`](math/021_pi-digits.bf.in) | 37 655 | 1 245 | 13 | 3,2·10¹⁰ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/pi-digits.bf) |
| 22 | [`PIdigits-as.b`](math/022_PIdigits-as.b) | π, вариант с другой арифметикой (30 знаков); вход: [`.in`](math/022_PIdigits-as.b.in) | 18 410 | 1 656 | 13 | 8,5·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/PIdigits-as.b) |
| 23 | [`e.bf`](math/023_e.bf) | число e без конца, Д. Кристофани; бесконечная, вход: [`.in`](math/023_e.bf.in) | 890 | 27 602 | 11 | ≥ 5,0·10⁹ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/e.bf) |
| 24 | [`calculate_e.bf`](math/024_calculate_e.bf) | число e, другой алгоритм; бесконечная, вход: [`.in`](math/024_calculate_e.bf.in) | 877 | 2 931 | 4 | ≥ 5,0·10⁹ | [4ffy/brainfuck-programs](https://github.com/4ffy/brainfuck-programs/blob/main/calculate_e.bf) |
| 25 | [`Golden.b`](math/025_Golden.b) | золотое сечение, 36 знаков | 1 950 | 382 | 17 | 8,8·10⁷ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Golden.b) |
| 26 | [`primes.bf`](math/026_primes.bf) | простые числа до 113; вход: [`.in`](math/026_primes.bf.in) | 1 305 | 12 | 6 | 3,7·10⁸ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/primes.bf) |
| 27 | [`Prime2.b`](math/027_Prime2.b) | простые числа до заданного предела; вход: [`.in`](math/027_Prime2.b.in) | 3 371 | 22 | 7 | 1,6·10⁶ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Prime2.b) |
| 28 | [`Prime-multi.b`](math/028_Prime-multi.b) | простые до 100, многоразрядная арифметика; вход: [`.in`](math/028_Prime-multi.b.in) | 12 368 | 19 | 9 | 1,8·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Prime-multi.b) |
| 29 | [`factor.bf`](math/029_factor.bf) | разложение на множители (362880); вход: [`.in`](math/029_factor.bf.in) | 3 818 | 108 | 15 | 1,6·10⁶ | [Wilfred/bfc](https://github.com/Wilfred/bfc/blob/master/sample_programs/factor.bf) |
| 30 | [`Collatz.b`](math/030_Collatz.b) | гипотеза Коллатца, Д. Кристофани; вход: [`.in`](math/030_Collatz.b.in) | 379 | 26 581 | 5 | 4,1·10⁹ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Collatz.b) |
| 31 | [`Euler1.b`](math/031_Euler1.b) | Project Euler №1 | 1 973 | 13 | 3 | 6,8·10⁵ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Euler1.b) |
| 32 | [`Euler5.b`](math/032_Euler5.b) | Project Euler №5; вход: [`.in`](math/032_Euler5.b.in) | 1 903 | 11 | 5 | 4,1·10⁵ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Euler5.b) |
| 33 | [`squaresums.b`](math/033_squaresums.b) | квадрат суммы минус сумма квадратов | 383 | 10 | 4 | 6,5·10⁶ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/squaresums.b) |
| 34 | [`fibonacci.bf`](math/034_fibonacci.bf) | числа Фибоначчи без конца, длинная арифметика; бесконечная | 171 | 7 798 | 13 | ≥ 5,0·10⁹ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/fibonacci.bf) |
| 35 | [`fibonacci-sequence-2.bf`](math/035_fibonacci-sequence-2.bf) | числа Фибоначчи, короткая версия | 96 | 6 | 2 | 5 886 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Fibonacci-sequence/Brainf---/fibonacci-sequence-2.bf) |
| 36 | [`factorial.bf`](math/036_factorial.bf) | факториалы без конца; бесконечная | 289 | 3 871 | 15 | ≥ 5,0·10⁹ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Factorial/Brainf---/factorial.bf) |
| 37 | [`thuemorse.bf`](math/037_thuemorse.bf) | последовательность Туэ — Морса; бесконечная | 64 | 32 | 3 | ≥ 5,0·10⁹ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/thuemorse.bf) |
| 38 | [`impeccable.bf`](math/038_impeccable.bf) | башни степеней двойки, Д. Кристофани; бесконечная, вход: [`.in`](math/038_impeccable.bf.in) | 211 | 17 600 | 9 | ≥ 5,0·10⁹ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/impeccable.bf) |
| 39 | [`random.bf`](math/039_random.bf) | генератор случайных чисел на клеточном автомате; бесконечная, **не помещается** | 111 | 58 276 | 6 | ≥ 5,0·10⁹ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/random.bf) |
| 40 | [`196.bf`](math/040_196.bf) | алгоритм 196: числа-палиндромы (89); вход: [`.in`](math/040_196.bf.in) | 1 163 | 74 | 6 | 1,6·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/196.bf) |
| 41 | [`binary-digits.bf`](math/041_binary-digits.bf) | числа в двоичной записи | 137 | 36 | 5 | 2,3·10⁶ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Binary-digits/Brainf---/binary-digits.bf) |
| 42 | [`count-in-octal.bf`](math/042_count-in-octal.bf) | счёт в восьмеричной системе | 143 | 16 | 5 | 1,3·10⁶ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Count-in-octal/Brainf---/count-in-octal.bf) |
| 43 | [`primality-by-trial-division-1.bf`](math/043_primality-by-trial-division-1.bf) | проверка числа на простоту; вход: [`.in`](math/043_primality-by-trial-division-1.bf.in) | 437 | 52 | 4 | 2,3·10⁵ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Primality-by-trial-division/Brainf---/primality-by-trial-division-1.bf) |
| 44 | [`luhn-test-of-credit-card-numbers.bf`](math/044_luhn-test-of-credit-card-numbers.bf) | контрольная сумма Луна; вход: [`.in`](math/044_luhn-test-of-credit-card-numbers.bf.in) | 488 | 144 | 4 | 67 089 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Luhn-test-of-credit-card-numbers/Brainf---/luhn-test-of-credit-card-numbers.bf) |
| 45 | [`divide.bf`](math/045_divide.bf) | деление с остатком; вход: [`.in`](math/045_divide.bf.in) | 386 | 16 | 5 | 1,1·10⁶ | [4ffy/brainfuck-programs](https://github.com/4ffy/brainfuck-programs/blob/main/divide.bf) |
| 46 | [`Calculator.bf`](math/046_Calculator.bf) | калькулятор выражений; вход: [`.in`](math/046_Calculator.bf.in) | 8 558 | 29 | 12 | 2 166 | [Shinbatsu/Brainfuck](https://github.com/Shinbatsu/Brainfuck/blob/main/BrainFuck/Calculator.bf) |
| 47 | [`abc.bf`](math/047_abc.bf) | распознавание языка aⁿbⁿcⁿ; вход: [`.in`](math/047_abc.bf.in) | 386 | 39 | 7 | 64 964 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/math/abc.bf) |
| 48 | [`fibint.b`](math/048_fibint.b) | Фибоначчи до переполнения 8-битной ячейки | 5 312 | 55 | 5 | 1,3·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/fibint.b) |
| 49 | [`main.bf`](math/049_main.bf) | двоичная дробь | 302 | 127 | 4 | 1,7·10⁵ | [erri4/Brainfuck](https://github.com/erri4/Brainfuck/blob/main/main.bf) |

### Текст

Утилиты потоковой обработки: rot13, wc, head, перекодировки, шифр Цезаря, разбор чисел.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 50 | [`rot13.bf`](text/050_rot13.bf) | ROT13, Д. Кристофани; вход: [`.in`](text/050_rot13.bf.in) | 179 | 9 | 5 | 1,8·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/rot13.bf) |
| 51 | [`rot13.bf`](text/051_rot13.bf) | ROT13, другая реализация; вход: [`.in`](text/051_rot13.bf.in) | 819 | 288 | 5 | 2,5·10⁷ | [4ffy/brainfuck-programs](https://github.com/4ffy/brainfuck-programs/blob/main/rot13.bf) |
| 52 | [`wc.bf`](text/052_wc.bf) | подсчёт строк, слов и символов (wc); вход: [`.in`](text/052_wc.bf.in) | 314 | 25 | 17 | 33 035 | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/wc.bf) |
| 53 | [`head.bf`](text/053_head.bf) | первые 10 строк (head); вход: [`.in`](text/053_head.bf.in) | 41 | 12 | 4 | 6 708 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/head.bf) |
| 54 | [`cat2.bf`](text/054_cat2.bf) | копирование ввода в вывод (cat); вход: [`.in`](text/054_cat2.bf.in) | 16 | 43 | 1 | 333 | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/cat2.bf) |
| 55 | [`reverse.bf`](text/055_reverse.bf) | переворот строки; вход: [`.in`](text/055_reverse.bf.in) | 13 | 42 | 1 | 328 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/reverse.bf) |
| 56 | [`dvorak.bf`](text/056_dvorak.bf) | перекодировка раскладки QWERTY → Dvorak; вход: [`.in`](text/056_dvorak.bf.in) | 762 | 514 | 3 | 1,4·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/dvorak.bf) |
| 57 | [`htmlconv.bf`](text/057_htmlconv.bf) | умляуты и спецсимволы → HTML; вход: [`.in`](text/057_htmlconv.bf.in) | 3 225 | 20 | 5 | 3,2·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/htmlconv.bf) |
| 58 | [`mac2unix.bf`](text/058_mac2unix.bf) | концы строк Mac → Unix; вход: [`.in`](text/058_mac2unix.bf.in) | 67 | 2 | 3 | 8 341 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/mac2unix.bf) |
| 59 | [`caesar-cipher-1.bf`](text/059_caesar-cipher-1.bf) | шифр Цезаря; вход: [`.in`](text/059_caesar-cipher-1.bf.in) | 632 | 15 | 6 | 1,1·10⁶ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Caesar-cipher/Brainf---/caesar-cipher-1.bf) |
| 60 | [`string-length.bf`](text/060_string-length.bf) | длина строки; вход: [`.in`](text/060_string-length.bf.in) | 207 | 28 | 4 | 5 559 | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/String-length/Brainf---/string-length.bf) |
| 61 | [`Kiloseconds.b`](text/061_Kiloseconds.b) | время суток в килосекундах; вход: [`.in`](text/061_Kiloseconds.b.in) | 1 675 | 23 | 67 | 18 500 | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Kiloseconds.b) |
| 62 | [`txtbf.b`](text/062_txtbf.b) | генератор Brainfuck-программы, печатающей текст; вход: [`.in`](text/062_txtbf.b.in) | 78 | 5 | 2 | 17 608 | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/extras/txtbf.b) |
| 63 | [`packbits.bf`](text/063_packbits.bf) | упаковка восьми битов в байт | 48 | 10 | 2 | 1 298 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/packbits.bf) |
| 64 | [`password-vault.bf`](text/064_password-vault.bf) | проверка пароля с ROT13; вход: [`.in`](text/064_password-vault.bf.in) | 3 623 | 9 | 52 | 8 090 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/password-vault.bf) |
| 65 | [`atoi.bf`](text/065_atoi.bf) | строка → число; вход: [`.in`](text/065_atoi.bf.in) | 153 | 7 | 4 | 4,9·10⁵ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/atoi.bf) |
| 66 | [`To_upper.bf`](text/066_To_upper.bf) | перевод в верхний регистр; вход: [`.in`](text/066_To_upper.bf.in) | 38 | 41 | 1 | 1 442 | [Shinbatsu/Brainfuck](https://github.com/Shinbatsu/Brainfuck/blob/main/BrainFuck/To_upper.bf) |

### Алгоритмы

Сортировки, Ханойские башни, универсальная машина Тьюринга, Busy Beaver.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 67 | [`bubblesort-1.bf`](algorithms/067_bubblesort-1.bf) | сортировка пузырьком; вход: [`.in`](algorithms/067_bubblesort-1.bf.in) | 117 | 85 | 4 | 2,1·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/sort/bubblesort-1.bf) |
| 68 | [`insertionsort.bf`](algorithms/068_insertionsort.bf) | сортировка вставками; вход: [`.in`](algorithms/068_insertionsort.bf.in) | 87 | 85 | 5 | 8,0·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/sort/insertionsort.bf) |
| 69 | [`quicksort.bf`](algorithms/069_quicksort.bf) | быстрая сортировка; вход: [`.in`](algorithms/069_quicksort.bf.in) | 179 | 129 | 5 | 1,3·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/sort/quicksort.bf) |
| 70 | [`sorting-algorithms-sleep-sort.bf`](algorithms/070_sorting-algorithms-sleep-sort.bf) | «сортировка сном»; вход: [`.in`](algorithms/070_sorting-algorithms-sleep-sort.bf.in) | 196 | 105 | 5 | 1,9·10⁶ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Sorting-algorithms-Sleep-sort/Brainf---/sorting-algorithms-sleep-sort.bf) |
| 71 | [`towers-of-hanoi.bf`](algorithms/071_towers-of-hanoi.bf) | Ханойские башни, текстом | 1 385 | 64 | 7 | 7,6·10⁵ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Towers-of-Hanoi/Brainf---/towers-of-hanoi.bf) |
| 72 | [`hanoi-opt.bf`](algorithms/072_hanoi-opt.bf) | Ханойские башни с анимацией, Clifford Wolf | 44 354 | 276 | 9 | 3,2·10⁸ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/hanoi-opt.bf) |
| 73 | [`utm.b`](algorithms/073_utm.b) | универсальная машина Тьюринга, Д. Кристофани; вход: [`.in`](algorithms/073_utm.b.in) | 461 | 395 | 24 | 1,9·10⁷ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/utm.b) |
| 74 | [`BusyBeaver.b`](algorithms/074_BusyBeaver.b) | «усердный бобёр» | 79 | 27 306 | 3 | 4,0·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/BusyBeaver.b) |
| 75 | [`2d_table.bf`](algorithms/075_2d_table.bf) | чтение двумерной таблицы | 381 | 44 | 3 | 1 036 | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/2d_table.bf) |
| 76 | [`numwarp.bf`](algorithms/076_numwarp.bf) | числа «искажёнными» цифрами, Д. Кристофани; вход: [`.in`](algorithms/076_numwarp.bf.in) | 860 | 371 | 23 | 2,3·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/numwarp.bf) |

### Интерпретаторы и компиляторы

Brainfuck на Brainfuck, трансляторы в C, компилятор awib, интерпретатор LISP — самые длинные и глубоко вложенные программы набора.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 77 | [`dbfi.bf`](interpreters/077_dbfi.bf) | Brainfuck на Brainfuck, Д. Кристофани; вход: [`.in`](interpreters/077_dbfi.bf.in) | 423 | 141 | 7 | 2,4·10⁶ | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/dbfi.bf) |
| 78 | [`cgbfi.bf`](interpreters/078_cgbfi.bf) | интерпретатор Clive Gifford; вход: [`.in`](interpreters/078_cgbfi.bf.in) | 930 | 238 | 9 | 8,3·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/interpreter/cgbfi.bf) |
| 79 | [`bfbf.bf`](interpreters/079_bfbf.bf) | интерпретатор BfBf; вход: [`.in`](interpreters/079_bfbf.bf.in) | 23 716 | 130 | 14 | 1,1·10⁹ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/interpreter/bfbf.bf) |
| 80 | [`bfi446.bf`](interpreters/080_bfi446.bf) | интерпретатор в 446 байт; вход: [`.in`](interpreters/080_bfi446.bf.in) | 440 | 346 | 8 | 3,3·10⁷ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/interpreter/bfi446.bf) |
| 81 | [`execute-brain--1.bf`](interpreters/081_execute-brain--1.bf) | интерпретатор с Rosetta Code; вход: [`.in`](interpreters/081_execute-brain--1.bf.in) | 6 142 | 543 | 14 | 1,3·10⁸ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Execute-Brain-/Brainf---/execute-brain--1.bf) |
| 82 | [`dbf2c.bf`](interpreters/082_dbf2c.bf) | компилятор Brainfuck → C; вход: [`.in`](interpreters/082_dbf2c.bf.in) | 890 | 20 | 15 | 64 254 | [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck/blob/master/programs/dbf2c.bf) |
| 83 | [`bf2c_v2.b`](interpreters/083_bf2c_v2.b) | компилятор Brainfuck → C, вариант; вход: [`.in`](interpreters/083_bf2c_v2.b.in) | 1 972 | 16 | 11 | 4,1·10⁵ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/extras/bf2c_v2.b) |
| 84 | [`bfc.bf`](interpreters/084_bfc.bf) | компилятор в DOS COM; вход: [`.in`](interpreters/084_bfc.bf.in) | 3 148 | 159 | 7 | 9,2·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/compiler/bfc.bf) |
| 85 | [`bfcl.bf`](interpreters/085_bfcl.bf) | компилятор в Linux ELF; вход: [`.in`](interpreters/085_bfcl.bf.in) | 3 563 | 138 | 16 | 1,7·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/compiler/bfcl.bf) |
| 86 | [`awib-0.4.b`](interpreters/086_awib-0.4.b) | awib — оптимизирующий компилятор; **не помещается**, вход: [`.in`](interpreters/086_awib-0.4.b.in) | 33 528 | 30 647 | 31 | 1,4·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/awib-0.4.b) |
| 87 | [`Zozotez.b`](interpreters/087_Zozotez.b) | интерпретатор LISP Zozotez; вход: [`.in`](interpreters/087_Zozotez.b.in) | 35 269 | 2 648 | 33 | 5,0·10⁸ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/Zozotez.b) |

### Квайны

Программы, печатающие свой текст: много вывода при небольшом коде.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 88 | [`quine-392.bf`](quines/088_quine-392.bf) | квайн в 392 команды | 397 | 785 | 3 | 7,9·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/quine/quine-392.bf) |
| 89 | [`quine-dc.bf`](quines/089_quine-dc.bf) | квайн Д. Кристофани; вход: [`.in`](quines/089_quine-dc.bf.in) | 892 | 206 | 3 | 5,2·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/quine/quine-dc.bf) |
| 90 | [`ryanquine.bf`](quines/090_ryanquine.bf) | квайн Ryan Kusnery | 1 566 | 652 | 3 | 8,3·10⁶ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/quine/ryanquine.bf) |
| 91 | [`quine-8780.bf`](quines/091_quine-8780.bf) | квайн в 8780 команд | 8 776 | 325 | 2 | 2,1·10⁵ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/quine/quine-8780.bf) |
| 92 | [`selfportrait.bf`](quines/092_selfportrait.bf) | квайн-автопортрет 132×48, Erik Bosman | 2 760 | 15 607 | 11 | 9,9·10⁷ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/quine/selfportrait.bf) |
| 93 | [`quine.bf`](quines/093_quine.bf) | квайн с Rosetta Code | 818 | 296 | 5 | 1,9·10⁵ | [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData/blob/main/Task/Quine/Brainf---/quine.bf) |

### Игры и графика

Текстовая игра Lost Kingdom, «Жизнь», множество Мандельброта.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 94 | [`LostKng.b`](games/094_LostKng.b) | текстовая игра Lost Kingdom, Jon Ripley; **не помещается**, вход: [`.in`](games/094_LostKng.b.in) | 2,1·10⁶ | 651 | 5 | 9,9·10⁷ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/LostKng.b) |
| 95 | [`life.bf`](games/095_life.bf) | игра «Жизнь», Д. Кристофани; вход: [`.in`](games/095_life.bf.in) | 2 391 | 987 | 7 | 8,8·10⁷ | [Wilfred/bfc](https://github.com/Wilfred/bfc/blob/master/sample_programs/life.bf) |
| 96 | [`mandel.b`](games/096_mandel.b) | множество Мандельброта, Erik Bosman | 11 203 | 308 | 9 | 1,1·10¹⁰ | [kostya/benchmarks](https://github.com/kostya/benchmarks/blob/master/brainfuck/mandel.b) |
| 97 | [`mandelbrot-huge.bf`](games/097_mandelbrot-huge.bf) | Мандельброт крупным планом | 11 219 | 308 | 9 | 5,7·10¹⁰ | [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck/blob/master/examples/mandelbrot/mandelbrot-huge.bf) |

### Нагрузочные тесты

Классические бенчмарки интерпретаторов и тест на всю ленту в 30 000 ячеек.

| № | Программа | Что делает | Команд | Ячеек | Loop | Инструкций | Источник |
|---:|---|---|---:|---:|---:|---:|---|
| 98 | [`bench.b`](benchmarks/098_bench.b) | эталонный тест скорости | 181 | 8 | 8 | 9,5·10⁸ | [kostya/benchmarks](https://github.com/kostya/benchmarks/blob/master/brainfuck/bench.b) |
| 99 | `long.b` | долгий счёт вложенными циклами | 164 | 42 | 9 | 7,9·10⁹ | [matslina/bfoptimization](https://github.com/matslina/bfoptimization/blob/master/progs/long.b) |
| 100 | [`cristofd-30000.b`](benchmarks/100_cristofd-30000.b) | проверка 30 000 ячеек ленты | 100 | 30 000 | 4 | 1,8·10⁷ | [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck/blob/master/testing/cristofd-30000.b) |

## Как запускать

Программы — исходные файлы как в источниках, с комментариями. Любой интерпретатор Brainfuck с 8-битными ячейками их выполнит.

Для `dpcrun` и загрузчика DekatronPC текст сначала нужно очистить до восьми символов Brainfuck: таблица символов DekatronPC считает опкодами ещё и буквы `N H 0 M G P D B E S L I A R r` и фигурные скобки, а они часто встречаются в комментариях.

```sh
tr -cd '+-<>[].,' < programs/bf100/math/Golden.b > /tmp/golden.bfk
bin/dpcrun -f /tmp/golden.bfk
```

## Источники и лицензии

Программы взяты из открытых репозиториев GitHub без изменений. Авторские права принадлежат авторам программ; автор обычно указан в комментарии в начале файла. Каждая программа распространяется на условиях репозитория, из которого она взята, тексты лицензий — в папке [`LICENSES/`](LICENSES). Лицензия bfutils (BSD-2-Clause) на эти файлы не распространяется.

| Источник | Программ | Лицензия |
|---|---:|---|
| [fabianishere/brainfuck](https://github.com/fabianishere/brainfuck) | 39 | [Apache-2.0](LICENSES/fabianishere_brainfuck.txt) |
| [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck) | 22 | [GPL-2.0](LICENSES/rdebath_Brainfuck.txt) |
| [acmeism/RosettaCodeData](https://github.com/acmeism/RosettaCodeData) | 15 | GFDL 1.2 (тексты Rosetta Code), решения задач с [Rosetta Code](https://rosettacode.org) |
| [pablojorge/brainfuck](https://github.com/pablojorge/brainfuck) | 9 | [MIT](LICENSES/pablojorge_brainfuck.txt) |
| [Wilfred/bfc](https://github.com/Wilfred/bfc) | 4 | [GPL-2.0](LICENSES/Wilfred_bfc.txt) |
| [4ffy/brainfuck-programs](https://github.com/4ffy/brainfuck-programs) | 4 | [0BSD](LICENSES/4ffy_brainfuck-programs.txt) |
| [Shinbatsu/Brainfuck](https://github.com/Shinbatsu/Brainfuck) | 2 | [MIT](LICENSES/Shinbatsu_Brainfuck.txt) |
| [kostya/benchmarks](https://github.com/kostya/benchmarks) | 2 | [MIT](LICENSES/kostya_benchmarks.txt) |
| [radiolok/dekatronpc](https://github.com/radiolok/dekatronpc) | 1 | — |
| [erri4/Brainfuck](https://github.com/erri4/Brainfuck) | 1 | [MIT](LICENSES/erri4_Brainfuck.txt) |
| [matslina/bfoptimization](https://github.com/matslina/bfoptimization) | 1 | не указана; файл не включён, только ссылка |

Программа `long.b` входит в набор, но файл не включён: у репозитория-источника нет лицензии. Метрики сняты, скачать программу можно по ссылке в таблице.

Набор собран в октябре 2026 года.

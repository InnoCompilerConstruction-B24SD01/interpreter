# Примеры запуска

Здесь собраны 15 успешных программ и 15 программ с намеренными ошибками.
У каждого примера приведены исходный код, команда запуска, вывод и код
завершения. Сообщения интерпретатора сохранены на английском, как в программе.

## Подготовка

Из корня репозитория соберите интерпретатор, затем перейдите в каталог примеров:

```sh
python3 verify.py --build-only
cd interpreter/examples
```

Все команды ниже выполняются из `interpreter/examples` и приведены для macOS.
На Linux замените `../../.build/darwin/dinterpret` на
`../../.build/linux/dinterpret`, на Windows — на
`../../.build/windows/dinterpret.exe`. Путь к программе также печатает сборка.

В диагностике ниже указан только файл, поскольку именно такое имя передаётся
команде. При запуске с другим путём префикс сообщения изменится.
`stdout` — стандартный вывод, `stderr` — поток ошибок. Если поток пуст,
это указано текстом, который сама программа не печатает.

Любой пример можно запустить с флагом `--optimize` перед именем файла.
Для примера 13 приведены оба варианта команды.

## Успешное выполнение

### 01. Динамические типы и none

Переменная без инициализатора содержит `none`. Присваивание меняет значение и его тип; `is` проверяет текущий тип.

Файл: [01_dynamic_types.d](01_dynamic_types.d).

```text
var x
print x is none
x := 7
print x, x is int
x := "seven"
print x, x is string
```

Запуск:

```sh
../../.build/darwin/dinterpret 01_dynamic_types.d
```

`stdout`:

```text
true
7 true
seven true
```

`stderr` пуст.

Код завершения: `0`.

### 02. Арифметика

Умножение выполняется раньше сложения. Целочисленное деление округляется вниз, а наличие вещественного операнда выбирает вещественное деление.

Файл: [02_arithmetic.d](02_arithmetic.d).

```text
print 2 + 3 * 4
print -7 / 2
print 7 / 2.0
```

Запуск:

```sh
../../.build/darwin/dinterpret 02_arithmetic.d
```

`stdout`:

```text
14
-4
3.5
```

`stderr` пуст.

Код завершения: `0`.

### 03. Ветвление if / else

Условие ложно, поэтому выполняется ветка `else`.

Файл: [03_if_else.d](03_if_else.d).

```text
var temperature := 18
if temperature >= 20 then
    print "warm"
else
    print "cold"
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 03_if_else.d
```

`stdout`:

```text
cold
```

`stderr` пуст.

Код завершения: `0`.

### 04. Цикл while

Условие вычисляется перед каждой итерацией. Присваивание уменьшает внешнюю переменную `n`.

Файл: [04_while.d](04_while.d).

```text
var n := 3
while n > 0 loop
    print n
    n := n - 1
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 04_while.d
```

`stdout`:

```text
3
2
1
```

`stderr` пуст.

Код завершения: `0`.

### 05. Бесконечный цикл и exit

Короткий `if` выполняет одну команду. При `n = 2` команда `exit` завершает ближайший цикл.

Файл: [05_loop_exit.d](05_loop_exit.d).

```text
var n := 0
loop
    n := n + 1
    print n
    if n = 2 => exit
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 05_loop_exit.d
```

`stdout`:

```text
1
2
```

`stderr` пуст.

Код завершения: `0`.

### 06. Диапазон for

Цикл включает обе границы: `2`, `3` и `4`.

Файл: [06_for_range.d](06_for_range.d).

```text
for n in 2..4 loop
    print n
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 06_for_range.d
```

`stdout`:

```text
2
3
4
```

`stderr` пуст.

Код завершения: `0`.

### 07. Разреженный массив

Запись по ключу создаёт отдельный элемент. Цикл читает значения по возрастанию ключей: сначала `10`, затем `1000`.

Файл: [07_sparse_array.d](07_sparse_array.d).

```text
var a := []
a[1000] := "last"
a[10] := "first"
for value in a loop
    print value
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 07_sparse_array.d
```

`stdout`:

```text
first
last
```

`stderr` пуст.

Код завершения: `0`.

### 08. Кортеж: имена и позиции полей

Запись в `point.x` меняет поле, доступное также через `point.1`. Позиции начинаются с `1`.

Файл: [08_tuple.d](08_tuple.d).

```text
var point := {x := 2, y := 3, "point"}
point.x := 5
print point.1, point.y, point.3
```

Запуск:

```sh
../../.build/darwin/dinterpret 08_tuple.d
```

`stdout`:

```text
5 3 point
```

`stderr` пуст.

Код завершения: `0`.

### 09. Две переменные с общим массивом

Присваивание копирует указатель на коллекцию. Изменение через `b` видно через `a`.

Файл: [09_collection_alias.d](09_collection_alias.d).

```text
var a := [1, 2]
var b := a
b[1] := 9
print a[1], b[1]
```

Запуск:

```sh
../../.build/darwin/dinterpret 09_collection_alias.d
```

`stdout`:

```text
9 9
```

`stderr` пуст.

Код завершения: `0`.

### 10. Функция как аргумент и элемент массива

Функция `apply` принимает другую функцию. Из массива извлекается функция возведения в квадрат и вызывается с аргументом `5`.

Файл: [10_higher_order.d](10_higher_order.d).

```text
var apply := func(f, x) => f(x)
var ops := [func(n) => n * n]
print apply(ops[1], 5)
```

Запуск:

```sh
../../.build/darwin/dinterpret 10_higher_order.d
```

`stdout`:

```text
25
```

`stderr` пуст.

Код завершения: `0`.

### 11. Рекурсия

Каждый вызов получает собственный параметр `n`. При нуле функция возвращает `1`, затем вычисляются произведения при возврате из рекурсии.

Файл: [11_factorial.d](11_factorial.d).

```text
var factorial := func(n) is
    if n = 0 => return 1
    return n * factorial(n - 1)
end
print factorial(5)
```

Запуск:

```sh
../../.build/darwin/dinterpret 11_factorial.d
```

`stdout`:

```text
120
```

`stderr` пуст.

Код завершения: `0`.

### 12. Замыкания

Каждый вызов `makeCounter` создаёт отдельную ячейку `value`. Возвращённая функция сохраняет эту ячейку и меняет её при следующих вызовах.

Файл: [12_closure.d](12_closure.d).

```text
var makeCounter := func(value) is
    return func(step) is
        value := value + step
        return value
    end
end
var first := makeCounter(10), second := makeCounter(100)
print first(1), first(2)
print second(1), first(1)
```

Запуск:

```sh
../../.build/darwin/dinterpret 12_closure.d
```

`stdout`:

```text
11 13
101 14
```

`stderr` пуст.

Код завершения: `0`.

### 13. Вызов с побочным эффектом и оптимизация

Умножение результата вызова на ноль не отменяет сам вызов: функция печатает `called`. С `--optimize` вывод остаётся таким же.

Файл: [13_optimized_call.d](13_optimized_call.d).

```text
var f := func() is
    print "called"
    return 7
end
print f() * 0
```

Запуск:

```sh
../../.build/darwin/dinterpret 13_optimized_call.d
../../.build/darwin/dinterpret --optimize 13_optimized_call.d
```

`stdout`:

```text
called
0
```

`stderr` пуст.

Код завершения: `0`.

### 16. Изменение массива во время обхода

Перед циклом создаётся снимок значений `[10, 20]`. Замена второго элемента и добавление третьего меняют исходный массив, но не последовательность текущего обхода.

Файл: [16_iteration_snapshot.d](16_iteration_snapshot.d).

```text
var values := [10, 20]
for value in values loop
    values[2] := 99
    values[3] := 30
    print value
end
print values
```

Запуск:

```sh
../../.build/darwin/dinterpret 16_iteration_snapshot.d
```

`stdout`:

```text
10
20
[10, 99, 30]
```

`stderr` пуст.

Код завершения: `0`.

### 17. Конкатенация и обход кортежа

Оператор `+` соединяет строки и создаёт новые внешние коллекции. Цикл по кортежу читает значения полей в порядке объявления.

Файл: [17_concatenation.d](17_concatenation.d).

```text
print "hello" + " " + "world"
print [1, 2] + [3]
var record := {name := "Ada"} + {score := 10}
for value in record loop
    print value
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 17_concatenation.d
```

`stdout`:

```text
hello world
[1, 2, 3]
Ada
10
```

`stderr` пуст.

Код завершения: `0`.

## Ошибки

Каждый пример этого раздела завершается с кодом `1`. Лексические,
синтаксические и семантические ошибки обнаруживаются до исполнения программы.
Ошибки выполнения могут возникнуть после уже выполненных команд.

### 14. Необъявленная переменная

Семантический анализ не находит объявление `missing`, поэтому выполнение не начинается. Объявите переменную до использования или исправьте её имя.

Файл: [14_unknown_variable.d](14_unknown_variable.d).

```text
print missing
```

Запуск:

```sh
../../.build/darwin/dinterpret 14_unknown_variable.d
```

`stdout` пуст.

`stderr`:

```text
14_unknown_variable.d: semantic error at 1:7: use of undeclared identifier 'missing'
```

Код завершения: `1`.

### 15. Деление на ноль

Ошибка возникает во время выполнения. Предыдущий `print` уже вывел `started`. Проверьте делитель и выполняйте деление только при ненулевом значении.

Файл: [15_division_by_zero.d](15_division_by_zero.d).

```text
print "started"
var divisor := 0
print 7 / divisor
```

Запуск:

```sh
../../.build/darwin/dinterpret 15_division_by_zero.d
```

`stdout`:

```text
started
```

`stderr`:

```text
15_division_by_zero.d: runtime error at 3:9: division by zero
```

Код завершения: `1`.

### 18. Недопустимый символ

Лексер не распознаёт `@`. Удалите символ или замените его допустимым выражением, например `print 1`.

Файл: [18_invalid_character.d](18_invalid_character.d).

```text
print @
```

Запуск:

```sh
../../.build/darwin/dinterpret 18_invalid_character.d
```

`stdout` пуст.

`stderr`:

```text
18_invalid_character.d: lexical error at 1:7: unexpected character '@'
```

Код завершения: `1`.

### 19. Пропущенный end

Парсер доходит до конца файла внутри условного блока. Добавьте `end` после команды `print`.

Файл: [19_missing_end.d](19_missing_end.d).

```text
if true then
    print "hello"
```

Запуск:

```sh
../../.build/darwin/dinterpret 19_missing_end.d
```

`stdout` пуст.

`stderr`:

```text
19_missing_end.d: syntax error at 3:1: expected 'end' after conditional statement; got EOF
```

Код завершения: `1`.

### 20. Повторное объявление

В одной области видимости нельзя дважды объявить `x`. Для изменения существующей переменной замените вторую строку на `x := 2`.

Файл: [20_duplicate_declaration.d](20_duplicate_declaration.d).

```text
var x := 1
var x := 2
```

Запуск:

```sh
../../.build/darwin/dinterpret 20_duplicate_declaration.d
```

`stdout` пуст.

`stderr`:

```text
20_duplicate_declaration.d: semantic error at 2:5: duplicate declaration of 'x' in the same scope; first variable is at 1:5
```

Код завершения: `1`.

### 21. Return вне функции

На верхнем уровне программы `return` недопустим. Поместите его в тело функции или используйте `print 1`, если нужно вывести значение.

Файл: [21_return_outside_function.d](21_return_outside_function.d).

```text
return 1
```

Запуск:

```sh
../../.build/darwin/dinterpret 21_return_outside_function.d
```

`stdout` пуст.

`stderr`:

```text
21_return_outside_function.d: semantic error at 1:1: 'return' is allowed only inside a function
```

Код завершения: `1`.

### 22. Неподходящие типы операндов

Из строки нельзя вычесть число. Если нужна арифметика, задайте `value` числом. Тип переменной проверяется при выполнении операции.

Файл: [22_invalid_operands.d](22_invalid_operands.d).

```text
var value := "hello"
print value - 1
```

Запуск:

```sh
../../.build/darwin/dinterpret 22_invalid_operands.d
```

`stdout` пуст.

`stderr`:

```text
22_invalid_operands.d: runtime error at 2:13: operator '-' does not accept string and int
```

Код завершения: `1`.

### 23. Число вместо логического условия

Условие должно иметь тип `bool`. Число `1` автоматически не преобразуется в `true`. Например, используйте условие `condition = 1`.

Файл: [23_non_boolean_condition.d](23_non_boolean_condition.d).

```text
var condition := 1
if condition then
    print "unreachable"
end
```

Запуск:

```sh
../../.build/darwin/dinterpret 23_non_boolean_condition.d
```

`stdout` пуст.

`stderr`:

```text
23_non_boolean_condition.d: runtime error at 2:4: if condition must be bool, got int
```

Код завершения: `1`.

### 24. Отсутствующий ключ массива

В массиве есть только ключ `1`. Прочитайте `values[1]` или сначала запишите значение по ключу `2`.

Файл: [24_missing_array_key.d](24_missing_array_key.d).

```text
var values := [10]
print values[2]
```

Запуск:

```sh
../../.build/darwin/dinterpret 24_missing_array_key.d
```

`stdout` пуст.

`stderr`:

```text
24_missing_array_key.d: runtime error at 2:13: array has no key 2
```

Код завершения: `1`.

### 25. Строковый индекс массива

Индекс массива должен быть целым числом. Для первого элемента задайте `index := 1`.

Файл: [25_invalid_array_index.d](25_invalid_array_index.d).

```text
var values := [10]
var index := "first"
print values[index]
```

Запуск:

```sh
../../.build/darwin/dinterpret 25_invalid_array_index.d
```

`stdout` пуст.

`stderr`:

```text
25_invalid_array_index.d: runtime error at 3:13: array index must be int, got string
```

Код завершения: `1`.

### 26. Отсутствующее поле кортежа

В кортеже объявлены только поля `x` и `y`. Обращайтесь к существующему полю или добавьте `z` при создании кортежа.

Файл: [26_missing_tuple_field.d](26_missing_tuple_field.d).

```text
var point := {x := 2, y := 3}
print point.z
```

Запуск:

```sh
../../.build/darwin/dinterpret 26_missing_tuple_field.d
```

`stdout` пуст.

`stderr`:

```text
26_missing_tuple_field.d: runtime error at 2:12: tuple has no field 'z'
```

Код завершения: `1`.

### 27. Попытка вызвать число

В переменной `value` лежит число, которое нельзя вызвать. Для вызова присвойте ей функцию; для вывода самого числа используйте `print value`.

Файл: [27_call_non_function.d](27_call_non_function.d).

```text
var value := 10
print value(5)
```

Запуск:

```sh
../../.build/darwin/dinterpret 27_call_non_function.d
```

`stdout` пуст.

`stderr`:

```text
27_call_non_function.d: runtime error at 2:12: call requires func, got int
```

Код завершения: `1`.

### 28. Неверное количество аргументов

Функция принимает один параметр, но получает два аргумента. Исправьте вызов на `twice(1)` или измените определение функции.

Файл: [28_argument_count.d](28_argument_count.d).

```text
var twice := func(n) => n * 2
print twice(1, 2)
```

Запуск:

```sh
../../.build/darwin/dinterpret 28_argument_count.d
```

`stdout` пуст.

`stderr`:

```text
28_argument_count.d: runtime error at 2:12: expected 1 argument(s), got 2
```

Код завершения: `1`.

### 29. Превышение глубины вызовов

Функция вызывает себя без условия остановки. При попытке начать 68-й активный вызов срабатывает ограничение. Добавьте условие завершения; для большой глубины вычисления используйте цикл.

Файл: [29_call_depth.d](29_call_depth.d).

```text
var recurse := func() => recurse()
print recurse()
```

Запуск:

```sh
../../.build/darwin/dinterpret 29_call_depth.d
```

`stdout` пуст.

`stderr`:

```text
29_call_depth.d: runtime error at 1:33: maximum call depth exceeded (67)
```

Код завершения: `1`.

### 30. Переполнение целого числа

К максимальному значению `int64_t` прибавляется единица. Ограничьте входные значения или проверьте границу перед сложением.

Файл: [30_integer_overflow.d](30_integer_overflow.d).

```text
var maximum := 9223372036854775807
print maximum + 1
```

Запуск:

```sh
../../.build/darwin/dinterpret 30_integer_overflow.d
```

`stdout` пуст.

`stderr`:

```text
30_integer_overflow.d: runtime error at 2:15: integer overflow in addition
```

Код завершения: `1`.

## Автоматическая проверка

Из корня репозитория выполните:

```sh
python3 verify.py
```

Проверка сравнивает код завершения, `stdout` и `stderr` для каждого `.d` с
ожидаемыми файлами `.out` и `.err`, с оптимизацией и без неё. Если `.out`
отсутствует, ожидается пустой `stdout`. Наличие `.err` означает ожидаемую
ошибку и код завершения `1`; без него ожидаются пустой `stderr` и код `0`.

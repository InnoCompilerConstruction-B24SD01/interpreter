# Примеры для защиты

Здесь 11 коротких программ по 3–9 строк: пять успешных и шесть с ошибками
выполнения. В каждом примере выделена одна основная идея. Все программы проходят
лексический, синтаксический и семантический анализ.

## Запуск

Все команды выполняются из корня репозитория, где находится `verify.py`:

```sh
python3 verify.py
```

Ожидаемый результат на macOS:

```text
Built .build/darwin/dinterpret
Passed 11 examples with and without optimization.
```

Затем запускайте отдельные файлы командами ниже. На Linux замените
`.build/darwin/dinterpret` на `.build/linux/dinterpret`, на Windows — на
`.build/windows/dinterpret.exe`. Как перейти в нужный каталог, описано
в [основном README](../../README.md).

На защите удобно сначала назвать ожидаемый результат, затем запустить файл
и показать участок кода исполнителя, который объясняет результат.
Начните с суммы и условия (01), затем покажите функцию и рекурсию (03),
замыкание (02), массив (04) и `while` (05). Ошибки разбирайте по одной (06–11).

`stdout` — стандартный вывод, `stderr` — поток ошибок. Пояснение «пуст» сама
программа не печатает. Путь в диагностике совпадает с аргументом команды.

## Успешные программы

### 01. Сумма покупок и скидка

**Что объяснить на защите:** Цикл берёт цены из массива и каждый раз меняет одну внешнюю переменную `total`. После цикла условие проверяет накопленную сумму и применяет скидку.

Сумма `10 + 20 + 30` равна `60`; после вычитания скидки получается `50`.

Файл: [01_sum_discount.d](01_sum_discount.d).

```text
var total := 0
for price in [10, 20, 30] loop
    total := total + price
end
print "before discount", total
if total >= 50 then
    total := total - 10
end
print "after discount", total
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/01_sum_discount.d
```

`stdout`:

```text
before discount 60
after discount 50
```

`stderr` пуст.

Код завершения: `0`.

### 02. Счётчик-замыкание

**Что объяснить на защите:** `makeCounter` возвращает функцию, которая сохраняет доступ к параметру `start`. Вызов `makeCounter` уже закончился, но ячейка остаётся доступной через замыкание.

Первый вызов увеличивает `10` до `11`, второй — до `12`. Повторный вызов не создаёт `start` заново.

Файл: [02_counter_closure.d](02_counter_closure.d).

```text
var makeCounter := func(start) is
    return func() is
        start := start + 1
        return start
    end
end
var counter := makeCounter(10)
print counter()
print counter()
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/02_counter_closure.d
```

`stdout`:

```text
11
12
```

`stderr` пуст.

Код завершения: `0`.

### 03. Факториал

**Что объяснить на защите:** Каждый вызов получает своё значение `n`. При `n = 0` функция возвращает `1`; затем при возврате из рекурсии вычисляются произведения.

Для `5` получается `5 * 4 * 3 * 2 * 1 = 120`. С `--optimize` вывод тот же.

Файл: [03_factorial.d](03_factorial.d).

```text
var factorial := func(n) is
    if n = 0 => return 1
    return n * factorial(n - 1)
end
print factorial(5)
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/03_factorial.d
```

`stdout`:

```text
120
```

`stderr` пуст.

Код завершения: `0`.

### 04. Снимок массива в for

**Что объяснить на защите:** До первой итерации `iterationValues` копирует значения массива в снимок `[10, 20]`. Тело меняет исходный массив, а цикл продолжает обходить снимок.

Цикл печатает `10` и `20`, хотя после первой итерации второй элемент исходного массива уже равен `99`. Последний `print` показывает изменённый массив.

Файл: [04_array_snapshot.d](04_array_snapshot.d).

```text
var values := [10, 20]
for value in values loop
    values[2] := 99
    print value
end
print values
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/04_array_snapshot.d
```

`stdout`:

```text
10
20
[10, 99]
```

`stderr` пуст.

Код завершения: `0`.

### 05. Повторная проверка условия while

**Что объяснить на защите:** Условие вычисляется перед каждой итерацией с текущим `n`. Тело увеличивает `n`, пока его квадрат не станет больше `20`.

При `n = 4` условие ещё истинно. После увеличения до `5` условие ложно, поэтому выводится `5 25`.

Файл: [05_square_search.d](05_square_search.d).

```text
var n := 1
while n * n <= 20 loop
    n := n + 1
end
print n, n * n
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/05_square_search.d
```

`stdout`:

```text
5 25
```

`stderr` пуст.

Код завершения: `0`.

## Ошибки выполнения

Каждая программа успевает что-то вывести, затем останавливается с кодом `1`.
Предыдущий вывод сохраняется. Это показывает, что ошибка возникает в исполнителе.

### 06. Отсутствующий элемент массива

**Что объяснить на защите:** В массиве есть ключи `1` и `2`. Первое чтение успешно, а при чтении ключа `3` исполнитель не находит элемент и выбрасывает `RuntimeError`.

Вывод `10` сохраняется. Чтобы убрать ошибку, читайте `values[2]` или сначала присвойте значение `values[3]`.

Файл: [06_missing_index.d](06_missing_index.d).

```text
var values := [10, 20]
print values[1]
print values[3]
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/06_missing_index.d
```

`stdout`:

```text
10
```

`stderr`:

```text
interpreter/examples/06_missing_index.d: runtime error at 3:13: array has no key 3
```

Код завершения: `1`.

### 07. Число умножается на строку

**Что объяснить на защите:** Типы значений переменных проверяются при выполнении операции. `price` содержит число, а `quantity` — строку, поэтому умножение недопустимо.

Строка `calculating` выводится до ошибки. Для исправления замените `"two"` на число `2`.

Файл: [07_invalid_operand.d](07_invalid_operand.d).

```text
var price := 10
var quantity := "two"
print "calculating"
print price * quantity
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/07_invalid_operand.d
```

`stdout`:

```text
calculating
```

`stderr`:

```text
interpreter/examples/07_invalid_operand.d: runtime error at 4:13: operator '*' does not accept int and string
```

Код завершения: `1`.

### 08. Отсутствующее поле кортежа

**Что объяснить на защите:** Кортеж содержит поля `x` и `y`. Чтение `x` успешно; при обращении к `z` исполнитель не находит поле.

Вывод `2` сохраняется. Для исправления читайте `point.y` или добавьте поле `z` при создании кортежа.

Файл: [08_missing_field.d](08_missing_field.d).

```text
var point := {x := 2, y := 3}
print point.x
print point.z
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/08_missing_field.d
```

`stdout`:

```text
2
```

`stderr`:

```text
interpreter/examples/08_missing_field.d: runtime error at 3:12: tuple has no field 'z'
```

Код завершения: `1`.

### 09. Деление на ноль внутри функции

**Что объяснить на защите:** Одна и та же функция сначала получает допустимый делитель, затем нулевой. Ошибка возникает при вычислении `/` в теле функции, поэтому позиция в сообщении указывает на первую строку.

Первый вызов возвращает `10`, второй завершается ошибкой. Перед делением можно добавить проверку `count = 0` в тело-блок функции и вернуть `none` для пустой выборки.

Файл: [09_division_by_zero.d](09_division_by_zero.d).

```text
var average := func(total, count) => total / count
print average(30, 3)
print average(0, 0)
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/09_division_by_zero.d
```

`stdout`:

```text
10
```

`stderr`:

```text
interpreter/examples/09_division_by_zero.d: runtime error at 1:44: division by zero
```

Код завершения: `1`.

### 10. Неверное число аргументов

**Что объяснить на защите:** При вызове исполнитель сравнивает число аргументов с числом параметров. Первый вызов передаёт оба числа, второй — только одно.

Первый вызов печатает `5`. Второй останавливается до исполнения тела функции. Для исправления передайте второй аргумент, например `add(2, 0)`.

Файл: [10_argument_count.d](10_argument_count.d).

```text
var add := func(a, b) => a + b
print add(2, 3)
print add(2)
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/10_argument_count.d
```

`stdout`:

```text
5
```

`stderr`:

```text
interpreter/examples/10_argument_count.d: runtime error at 3:10: expected 2 argument(s), got 1
```

Код завершения: `1`.

### 11. Граница глубины рекурсии

**Что объяснить на защите:** Лимит равен 67 одновременно активным вызовам. Для `sumTo(66)` нужны вызовы от `66` до `0`, то есть ровно 67. Для `sumTo(67)` требуется 68, и последний вызов отклоняется.

Первая сумма равна `2211` и печатается. Вторая строка и `complete` не выводятся. У рекурсии есть условие остановки, но лимит достигается раньше. Для больших сумм используйте цикл или формулу.

Файл: [11_recursion_limit.d](11_recursion_limit.d).

```text
var sumTo := func(n) is
    if n = 0 => return 0
    return n + sumTo(n - 1)
end
print "sum 1..66", sumTo(66)
print "sum 1..67", sumTo(67)
print "complete"
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/11_recursion_limit.d
```

`stdout`:

```text
sum 1..66 2211
```

`stderr`:

```text
interpreter/examples/11_recursion_limit.d: runtime error at 3:21: maximum call depth exceeded (67)
```

Код завершения: `1`.

## Где показать реализацию

| Пример | Функции исполнителя |
| --- | --- |
| 01 | `executeFor`, `assign`, ветка `IfStatement` в `executeStatement` |
| 02 | Ветка `FunctionLiteral` в `evaluateExpression`, `callFunction`, `Environment::lookup` |
| 03 | `callFunction`, обработка `ReturnStatement` |
| 04 | `iterationValues`, `executeFor` |
| 05 | `executeLoop` |
| 06 | `arrayLocation`, ветка `IndexExpression` в `evaluateExpression` |
| 07 | `binaryOperation` |
| 08 | `tupleElement` |
| 09 | Проверка делителя в `arithmetic` |
| 10–11 | Проверки количества аргументов и глубины в `callFunction` |

Команды и циклы находятся в [interpreter.cpp](../src/interpreter.cpp),
выражения и вызовы — в [expressions.cpp](../src/expressions.cpp),
окружения — в [environment.hpp](../src/environment.hpp),
арифметика — в [value_operations.cpp](../src/value_operations.cpp).

## Проверка ожидаемых результатов

Файлы `.out` содержат ожидаемый `stdout`. Файлы `.err` содержат ожидаемую
диагностику и означают код завершения `1`; без `.err` ожидаются пустой `stderr`
и код `0`.

`verify.py` запускает каждый пример с оптимизацией и без неё и сравнивает
`stdout`, `stderr` и код завершения. Программы запускаются из каталога
`interpreter/examples`, поэтому в `.err` указано только имя файла, без префикса
`interpreter/examples/`, который присутствует в командах выше.

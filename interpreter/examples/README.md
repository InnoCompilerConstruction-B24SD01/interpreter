# Десять примеров запуска

Пять программ завершаются успешно, ещё пять показывают ошибки выполнения
(runtime errors). Все десять проходят лексический, синтаксический и семантический анализ. Каждый пример — небольшой сценарий с несколькими конструкциями языка.
Сообщения интерпретатора приведены дословно на английском.

## Подготовка

Все команды ниже выполняются из корня репозитория, где лежит `verify.py`:

```sh
python3 verify.py
```

На macOS результат успешной сборки и проверки:

```text
Built .build/darwin/dinterpret
Passed 10 examples with and without optimization.
```

Затем можно выполнить любой пример отдельной командой. На Linux замените
`.build/darwin/dinterpret` на `.build/linux/dinterpret`, на Windows — на
`.build/windows/dinterpret.exe`. Подробный порядок перехода в нужный каталог
описан в [основном README](../../README.md).

Ниже `stdout` означает стандартный вывод, `stderr` — поток ошибок. Фразы
о пустом потоке служат пояснениями и самой программой не печатаются.
В сообщении об ошибке путь совпадает с аргументом команды. Если запускать
файл из другого каталога с другим путём, изменится префикс сообщения.

## Успешные программы

### 01. Корзина со скидкой и печать чека

Корзина хранит кортежи товаров в массиве. Цикл считает стоимость каждой позиции и общую сумму. При сумме от `100` применяется скидка `10 %`. Чек собирается в кортеж: первое поле читается по позиции, остальные — по имени.

Файл: [01_shopping_cart.d](01_shopping_cart.d).

```text
var cart := [{name := "tea", price := 10, quantity := 3}, {name := "bread", price := 20, quantity := 2}, {name := "coffee", price := 50, quantity := 1}]
var subtotal := 0
for item in cart loop
    var cost := item.price * item.quantity
    subtotal := subtotal + cost
    print item.name, cost
end
var discount := 0
if subtotal >= 100 then
    discount := subtotal * 0.1
else
    print "no discount"
end
var receipt := {subtotal := subtotal, discount := discount, total := subtotal - discount}
print "subtotal", receipt.1
print "discount", receipt.discount
print "total", receipt.total
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/01_shopping_cart.d
```

`stdout`:

```text
tea 30
bread 40
coffee 50
subtotal 120
discount 12
total 108
```

`stderr` пуст.

Код завершения: `0`.

### 02. Независимые счета и замыкания

Каждый вызов `makeAccount` создаёт собственную ячейку `balance`. Возвращённая функция меняет её и сохраняет между вызовами. Одноимённая глобальная переменная остаётся равной `1000`. Переменная `report` проходит через `none`, число и строку; проверки `is` показывают текущий тип.

Файл: [02_account_closures.d](02_account_closures.d).

```text
var balance := 1000
var makeAccount := func(initial) is
    var balance := initial
    return func(delta) is
        balance := balance + delta
        return balance
    end
end
var first := makeAccount(10), second := makeAccount(100)
print "first", first(5), first(2)
print "second", second(-20)
print "outer", balance
var report
print "empty report", report is none
report := first(3)
print "balance", report, report is int
report := "account updated"
print report, report is string
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/02_account_closures.d
```

`stdout`:

```text
first 15 17
second 80
outer 1000
empty report true
balance 20 true
account updated true
```

`stderr` пуст.

Код завершения: `0`.

### 03. Рекурсивная обработка массива и функции как значения

Функция `map` принимает массив и функцию обработки. Факториал вычисляется рекурсивно, а квадрат — функцией с телом-выражением. Обе функции хранятся в массиве `operations`. Цикл по диапазону считает сумму факториалов. Вызов `audit()` печатает эту сумму даже при умножении результата на ноль, в том числе с оптимизацией.

Файл: [03_recursive_map.d](03_recursive_map.d).

```text
var factorial := func(n) is
    if n = 0 => return 1
    return n * factorial(n - 1)
end
var map := func(values, transform) is
    var result := []
    var index := 1
    for value in values loop
        result[index] := transform(value)
        index := index + 1
    end
    return result
end
var operations := [factorial, func(n) => n * n]
var input := [3, 4, 5]
print "factorials", map(input, operations[1])
print "squares", map(input, operations[2])
var total := 0
for n in 1..5 loop
    total := total + factorial(n)
end
var audit := func() is
    print "factorial sum", total
    return total
end
print "ignored result", audit() * 0
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/03_recursive_map.d
```

Тот же результат с оптимизацией:

```sh
.build/darwin/dinterpret --optimize interpreter/examples/03_recursive_map.d
```

`stdout`:

```text
factorials [6, 24, 120]
squares [9, 16, 25]
factorial sum 153
ignored result 0
```

`stderr` пуст.

Код завершения: `0`.

### 04. Изменение очереди во время обхода

Разреженный массив содержит задачи по ключам `10` и `30`, хотя добавлены они в обратном порядке. Переменная `alias` ссылается на тот же массив. Во время обхода задача по ключу `30` заменяется, а по ключу `50` добавляется новая. Цикл продолжает читать заранее созданный снимок и выполняет только `compile` и `test`. Затем программа печатает изменённую очередь, соединяет массивы и обходит объединённый кортеж итогов.

Файл: [04_queue_snapshot.d](04_queue_snapshot.d).

```text
var jobs := []
jobs[30] := "test"
jobs[10] := "compile"
var alias := jobs
var completed := []
var count := 0
for job in jobs loop
    alias[30] := "retest"
    alias[50] := "package"
    count := count + 1
    completed[count] := job
    print "running " + job
end
print "current queue", jobs
print "completed", completed + ["done"]
var summary := {processed := count} + {queued := 3}
for value in summary loop
    print "summary", value
end
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/04_queue_snapshot.d
```

`stdout`:

```text
running compile
running test
current queue [compile, retest, package]
completed [compile, test, done]
summary 2
summary 3
```

`stderr` пуст.

Код завершения: `0`.

### 05. Алгоритм Евклида и поиск взаимно простых чисел

Функция `gcd` вычисляет НОД в цикле `while`. Для положительных чисел остаток выражается через целочисленное деление: `a - (a / b) * b`. Внешний `loop` ищет первые четыре положительных числа, взаимно простых с `30`. Короткий `if` завершает поиск через `exit`, а функция возвращает НОД через `return`.

Файл: [05_coprime_search.d](05_coprime_search.d).

```text
var gcd := func(a, b) is
    while b /= 0 loop
        var remainder := a - (a / b) * b
        a := b
        b := remainder
    end
    return a
end
print "gcd", gcd(48, 18)
var candidate := 1
var found := 0
loop
    if gcd(candidate, 30) = 1 then
        print "coprime", candidate
        found := found + 1
    end
    if found = 4 => exit
    candidate := candidate + 1
end
print "checked through", candidate
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/05_coprime_search.d
```

`stdout`:

```text
gcd 6
coprime 1
coprime 7
coprime 11
coprime 13
checked through 13
```

`stderr` пуст.

Код завершения: `0`.

## Ошибки выполнения

Каждая программа начинает выполняться и успевает вывести результат до ошибки.
Затем исполнитель останавливается с кодом `1`; предыдущий вывод сохраняется.
В каждом сценарии выполняется отдельный запуск.

### 06. Товар отсутствует в складских остатках

В разреженном массиве `stock` задан остаток только для товара `100`, а запрос содержит также товар `200`. Первая строка остатков успешно печатается. При чтении второго ключа возникает ошибка выполнения. Добавьте остаток `stock[200] := 0` или передавайте в обход только ключи, для которых остатки заданы.

Файл: [06_missing_stock.d](06_missing_stock.d).

```text
var stock := []
stock[100] := 5
var requested := [100, 200]
print "inventory started"
for product in requested loop
    print "available", product, stock[product]
end
print "inventory complete"
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/06_missing_stock.d
```

`stdout`:

```text
inventory started
available 100 5
```

`stderr`:

```text
interpreter/examples/06_missing_stock.d: runtime error at 6:38: array has no key 200
```

Код завершения: `1`.

### 07. Строка вместо количества товара

В первой позиции количество задано числом, во второй — строкой `"two"`. Семантический анализ допускает обращение к динамическим полям. При вычислении второй позиции `lineTotal` пытается умножить цену на строку и завершается ошибкой. Исправьте количество на число `2` или проверяйте `item.quantity is int` до вычисления стоимости.

Файл: [07_invalid_quantity.d](07_invalid_quantity.d).

```text
var lineTotal := func(item) is
    return item.price * item.quantity
end
var cart := [{name := "tea", price := 10, quantity := 2}, {name := "coffee", price := 50, quantity := "two"}]
print "checkout started"
for item in cart loop
    print item.name, lineTotal(item)
end
print "checkout complete"
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/07_invalid_quantity.d
```

`stdout`:

```text
checkout started
tea 20
```

`stderr`:

```text
interpreter/examples/07_invalid_quantity.d: runtime error at 2:23: operator '*' does not accept int and string
```

Код завершения: `1`.

### 08. У посылки отсутствует вес

Функция `shippingCost` получает вложенный кортеж `package` и читает поле `weight`. У первого заказа поле есть, поэтому стоимость доставки печатается. У второго заказа указан только признак `fragile`: чтение отсутствующего поля вызывает ошибку выполнения. Добавьте `weight` при создании второго кортежа и передавайте функции посылки с одинаковым набором обязательных полей.

Файл: [08_missing_package_field.d](08_missing_package_field.d).

```text
var shippingCost := func(package) is
    return package.weight * 5
end
var orders := [{id := 101, package := {weight := 2, fragile := false}}, {id := 102, package := {fragile := true}}]
print "shipping started"
for order in orders loop
    print "order", order.id, shippingCost(order.package)
end
print "shipping complete"
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/08_missing_package_field.d
```

`stdout`:

```text
shipping started
order 101 10
```

`stderr`:

```text
interpreter/examples/08_missing_package_field.d: runtime error at 2:19: tuple has no field 'weight'
```

Код завершения: `1`.

### 09. Среднее для пустой группы

Первая группа успешно обрабатывается и печатается. Во второй группе нет значений, а `count` равен нулю. Деление внутри `average` вызывает ошибку; накопленный вывод сохраняется. Строка с именем второй группы не печатается, поскольку её `print` ещё вычисляет аргументы. Перед делением проверьте количество значений и определите результат для пустой группы, например `if count = 0 => return none`.

Файл: [09_empty_batch.d](09_empty_batch.d).

```text
var average := func(values, count) is
    var total := 0
    for value in values loop
        total := total + value
    end
    return total / count
end
var batches := [{name := "morning", values := [10, 20, 30], count := 3}, {name := "evening", values := [], count := 0}]
print "report started"
for batch in batches loop
    print batch.name, average(batch.values, batch.count)
end
print "report complete"
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/09_empty_batch.d
```

`stdout`:

```text
report started
morning 20
```

`stderr`:

```text
interpreter/examples/09_empty_batch.d: runtime error at 6:18: division by zero
```

Код завершения: `1`.

### 10. Неверная сигнатура функции обработки цен

`applyPrices` вызывает переданную функцию с одним аргументом — ценой. Функция `withTax` требует два аргумента: цену и ставку. Ошибка возникает при вложенном вызове после строки `pricing started`. Можно передать адаптер, который фиксирует ставку: `applyPrices([100, 200], func(price) => withTax(price, 0.2))`.

Файл: [10_callback_arguments.d](10_callback_arguments.d).

```text
var applyPrices := func(prices, transform) is
    var total := 0
    for price in prices loop
        total := total + transform(price)
    end
    return total
end
var withTax := func(price, rate) => price + price * rate
print "pricing started"
print "total", applyPrices([100, 200], withTax)
```

Запуск:

```sh
.build/darwin/dinterpret interpreter/examples/10_callback_arguments.d
```

`stdout`:

```text
pricing started
```

`stderr`:

```text
interpreter/examples/10_callback_arguments.d: runtime error at 4:35: expected 2 argument(s), got 1
```

Код завершения: `1`.

## Ожидаемые результаты

Файлы `.out` содержат ожидаемый `stdout`. Если `.out` отсутствует, поток должен
быть пустым. Файл `.err` содержит ожидаемую диагностику и означает код
завершения `1`. Без него ожидаются пустой `stderr` и код `0`.

`verify.py` запускает программы из каталога `interpreter/examples`, передавая
только имя файла. Поэтому в `.err` нет префикса `interpreter/examples/`,
который присутствует в приведённых выше запусках из корня.

Проверка выполняет каждый пример с оптимизацией и без неё и сравнивает все
три результата: `stdout`, `stderr` и код завершения.

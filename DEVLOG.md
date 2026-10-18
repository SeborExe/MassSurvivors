Day 1



* encja = handle (index + serial), dane leżą w chunkach archetypu,
* const shared fragment: wartość per chunk, deduplikacja po hashu wartości, różne wartości = osobne chunki w tym samym archetypie,
* przewidziałem układ pamięci (136 B/encję, 963/chunk, 6 → 5 chunków) i debugger to potwierdził,
* trade-off: wiele unikalnych configów z małą liczbą encji = zmarnowana pamięć.



Day 2



* procesor bez query może zostać pominięty przez query-based pruning; rozwiązanie: ShouldAllowQueryBasedPruning() → false,
* deklaracje dostępu chronią przed równoległym wykonaniem, ale kolejność ustala się jawnie (ExecuteAfter/ExecuteBefore),
* debugowanie łańcucha procesorów: sprawdzanie danych pośrednich (prędkość) w debuggerze, żeby zawęzić, gdzie leży błąd.



Day 3



* statystyka mierzy tylko to, co oznaczysz; gdy stat i Insights się nie zgadzają, ufaj Insights,
* „Game” w stat unit obejmuje też czekanie na workery Mass,
* subsystem z kontekstu jest dostępny tylko w trakcie iteracji query,
* budowy gridu nie można zrównoleglić (wspólna struktura), a separację można (każdy chunk pisze tylko do swoich danych).



Day 4






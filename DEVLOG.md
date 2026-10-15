Day 1



* encja = handle (index + serial), dane leżą w chunkach archetypu,
* const shared fragment: wartość per chunk, deduplikacja po hashu wartości, różne wartości = osobne chunki w tym samym archetypie,
* przewidziałem układ pamięci (136 B/encję, 963/chunk, 6 → 5 chunków) i debugger to potwierdził,
* trade-off: wiele unikalnych configów z małą liczbą encji = zmarnowana pamięć.



Day 2






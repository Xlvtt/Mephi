from collections.abc import Callable
from functools import wraps


def lru_cache(max_items: int) -> Callable:
    """
    Функция создает декоратор, позволяющий кэшировать результаты выполнения обернутой функции по принципу LRU-кэша.
    Размер LRU кэша ограничен количеством max_items. При попытке сохранить новый результат в кэш, в том случае, когда
    размер кэша уже равен max_size, происходит удаление одного из старых элементов, сохраненных в кэше.
    Удаляется тот элемент, к которому обращались давнее всего.
    Также у обернутой функции должен появиться атрибут stats, в котором лежит объект с атрибутами cache_hits и
    cache_misses, подсчитывающие количество успешных и неуспешных использований кэша.
    :param max_items: максимальный размер кэша.
    :return: декоратор, добавляющий LRU-кэширование для обернутой функции.
    """

    class Stats:
        cache_hits = 0
        cache_misses = 0

    def decorator(f):
        cache = {}
        lst = []
        f.stats = Stats()

        @wraps(f)
        def cache_wrapper(*args, **kwargs):
            key = str(args) + str(kwargs)
            link = cache.get(key)
            if link is not None:
                lst.remove(key)
                lst.append(key)
                f.stats.cache_hits += 1
                return link
            else:
                f.stats.cache_misses += 1
                if len(lst) == max_items:
                    old_key = lst.pop(0)
                    # cache.pop(old_key)
                    del cache[old_key]
                res = f(*args, **kwargs)
                cache[key] = res
                lst.append(key)
                return res

        return cache_wrapper

    return decorator
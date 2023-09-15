from collections.abc import Iterable, Generator


def flatten(iterable: Iterable) -> Generator:
    """
    Генератор flatten принимает итерируемый объект iterable и с помощью обхода в глубину отдает все вложенные объекты.
    Для любых итерируемых вложенных объектов, не являющихся строками, нужно делать рекурсивный заход.
    В результате генератор должен пробегать по всем вложенным объектам на любом уровне вложенности.
    """
    for now in iterable:
        if isinstance(now, Iterable) and not isinstance(now, str):
            for el in flatten(now):
                yield el
        else:
            yield now

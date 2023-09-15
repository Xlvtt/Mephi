class EnhancedList(list):
    def __init__(self, lst: list=[]):
        super().__init__(lst)

    @property #создать свойство first
    def first(self) -> object:
        return self[0]

    @first.setter #то же что first = first.setter(first)
    def first(self, val: object):
        self[0] = val

    @property
    def last(self) -> object:
        return self[-1]

    @last.setter
    def last(self, val: object):
        self[-1] = val

    @property
    def size(self) -> int:
        return len(self)

    @size.setter
    def size(self, new_size: int):
        if new_size < self.size:
            for i in range(self.size - new_size):
                self.pop()
        else:
            for i in range(new_size - self.size):
                self.append(None)

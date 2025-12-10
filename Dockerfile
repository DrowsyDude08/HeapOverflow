FROM ubuntu:22.04

# Устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y \
    g++ \
    gcc \
    make \
    libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Создаем рабочую директорию
WORKDIR /app

# Копируем исходный код
COPY vulnerable_heap_overflow.cpp .

# Компилируем программу
# Отключаем защиту для демонстрации уязвимости
# -fno-stack-protector: отключает защиту стека
# -no-pie: отключает Position Independent Executable
# -z execstack: разрешает выполнение кода в стеке (для демонстрации)
RUN g++ -o vulnerable_heap_overflow vulnerable_heap_overflow.cpp \
    -fno-stack-protector \
    -no-pie \
    -z execstack

# Создаем непривилегированного пользователя
RUN useradd -m -u 1000 appuser && chown -R appuser:appuser /app

# Переключаемся на непривилегированного пользователя
USER appuser

# Открываем порт
EXPOSE 1337

# Запускаем приложение
CMD ["./vulnerable_heap_overflow", "1337"]


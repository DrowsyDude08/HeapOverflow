#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Структура для хранения пользовательских данных
// УЯЗВИМОСТЬ: буфер находится ВНУТРИ структуры, переполнение перезапишет другие поля!
#pragma pack(1)  // Отключаем выравнивание структуры (padding) для предсказуемости смещений
struct UserData {
    char buffer[32];        // Буфер для пользовательского ввода (в начале структуры)
    char username[32];
    char password[32];
    int is_admin;            // Цель эксплуатации - установить в 1
    char secret_flag[128];
};
#pragma pack()  // Возвращаем стандартное выравнивание

// Функция получения флага из переменной окружения
const char* get_flag() {
    const char* flag = std::getenv("FLAG");
    if (flag == nullptr) {
        return "practice{default_flag_not_set}";
    }
    return flag;
}

// Уязвимая функция с переполнением кучи
void vulnerable_heap_operation(int client_socket, const char* input) {
    // Выделяем структуру UserData в куче
    // Структура содержит буфер внутри себя
    UserData* user = (UserData*)malloc(sizeof(UserData));
    if (!user) {
        const char* error = "Memory allocation failed!\n";
        send(client_socket, error, strlen(error), 0);
        return;
    }
    
    // Инициализируем структуру
    memset(user, 0, sizeof(UserData));
    user->is_admin = 0;
    
    // Копируем флаг в секретное поле структуры
    strncpy(user->secret_flag, get_flag(), sizeof(user->secret_flag) - 1);
    user->secret_flag[sizeof(user->secret_flag) - 1] = '\0';
    
    // КРИТИЧЕСКАЯ УЯЗВИМОСТЬ: Heap Overflow
    // memcpy копирует данные БЕЗ проверки размера буфера
    // Буфер находится ВНУТРИ структуры, поэтому переполнение перезапишет:
    // - username (32 байта после buffer)
    // - password (32 байта после username)
    // - is_admin (4 байта после password) ← ЦЕЛЬ!
    // Используем strlen для определения длины, но не проверяем границы буфера!
    size_t input_len = strlen(input);
    memcpy(user->buffer, input, input_len + 1);  // ← Здесь происходит переполнение кучи (+1 для null terminator)
    
    // Проверяем права администратора
    if (user->is_admin == 1) {
        const char* admin_msg = "ADMIN ACCESS GRANTED!\n";
        send(client_socket, admin_msg, strlen(admin_msg), 0);
        
        char flag_msg[256];
        snprintf(flag_msg, sizeof(flag_msg), "Congratulations! Here's your flag: %s\n", user->secret_flag);
        send(client_socket, flag_msg, strlen(flag_msg), 0);
    } else {
        const char* normal_msg = "Access denied. Regular user.\n";
        send(client_socket, normal_msg, strlen(normal_msg), 0);
    }
    
    // Освобождаем память
    free(user);
}

void handle_client(int client_socket) {
    char buffer[2048];
    const char* welcome = "=== Heap Overflow CTF Challenge ===\n"
                          "Enter your data (will be stored in heap):\n";
    
    send(client_socket, welcome, strlen(welcome), 0);
    
    // Читаем данные от клиента
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        
        // Вызываем уязвимую функцию
        vulnerable_heap_operation(client_socket, buffer);
    }
    
    close(client_socket);
}

int main(int argc, char* argv[]) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int port = 1337;
    
    // Парсим порт из аргументов или используем по умолчанию
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    // Создаем сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Устанавливаем опции сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Привязываем сокет к адресу
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Слушаем входящие соединения
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Сервер запущен на порту " << port << std::endl;
    std::cout << "Подключитесь через: nc <host> " << port << std::endl;
    
    // Принимаем соединения
    while (true) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Новое соединение от " << inet_ntoa(address.sin_addr) << std::endl;
        
        handle_client(client_socket);
    }
    
    return 0;
}


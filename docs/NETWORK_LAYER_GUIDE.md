# Network Layer Implementation Guide

## Overview

Это руководство описывает минимально компилируемую версию Network слоя IRC сервера. Версия содержит основные компоненты для управления сокетами, клиентами и событиями I/O.

## Структура Network Слоя

```
Network Layer (Alex)
├── Server         - главный класс для управления сервером
├── Poller         - обработка I/O событий через poll()
├── Config         - конфигурация сервера (порт, пароль)
├── Client         - состояние клиента (пока заглушка)
└── MessageBuffer  - буфер для частичных сообщений (пока заглушка)
```

## Ключевые компоненты

### 1. Config (src/Config.cpp, include/irc/Config.hpp)

**Назначение:** Управление конфигурацией сервера

**Основные методы:**
```cpp
Config(int port, const std::string& password);  // Конструктор
int getPort() const;                            // Получить порт
const std::string& getPassword() const;         // Получить пароль
Config parseArgs(int argc, char** argv);        // Парсинг аргументов командной строки
```

**Как работает:**
```bash
./ircserv 6667 mypassword
# parseArgs() извлечет порт 6667 и пароль mypassword
```

### 2. Poller (src/Poller.cpp, include/irc/Poller.hpp)

**Назначение:** Единственный класс, вызывающий poll() для мониторинга файловых дескрипторов

**КРИТИЧЕСКОЕ ПРАВИЛО:** Только Poller вызывает poll()!

**Основные методы:**
```cpp
Poller(Server* server);              // Конструктор, сохраняет указатель на Server
void addFd(int fd, short events);    // Добавить fd для мониторинга (POLLIN, POLLOUT)
void removeFd(int fd);               // Удалить fd из мониторинга
int poll(int timeout = -1);          // Вызвать poll(), вернуть готовые fd
void processEvents();                // Обработать события (вызывает методы Server)
```

**Жизненный цикл:**
1. Server добавляет server socket fd в Poller
2. Poller держит список файловых дескрипторов в `pollfds_`
3. poll() вызывается раз в основном цикле
4. processEvents() обходит результаты и вызывает нужные методы Server

### 3. Server (src/Server.cpp, include/irc/Server.hpp)

**Назначение:** Координирует все компоненты, управляет сокетами и клиентами

**Основные методы:**

#### Инициализация:
```cpp
Server(const Config& config);  // Конструктор
~Server();                     // Деструктор, очистка ресурсов
void start();                  // Создать сокет, bind, listen
void run();                    // Основной цикл обработки событий
```

#### Управление соединениями:
```cpp
void handleNewConnection();         // [TODO] Принять новое соединение
void handleClientInput(int fd);     // [TODO] Читать данные от клиента
void disconnectClient(int fd);      // [TODO] Отключить клиента
```

#### Отправка данных:
```cpp
void sendToClient(int fd, const std::string& msg);  // [TODO] Отправить сообщение
void sendResponse(...);                              // [TODO] Отправить IRC ответ
void broadcastToChannel(...);                        // [TODO] Broadcast в канал
```

#### Управление клиентами:
```cpp
Client* getClient(int fd);                   // Получить клиента по fd
// Client* getClientByNickname(...);         // [TODO] Получить по нику
// void addClient(...);                      // [TODO] Добавить клиента
// void removeClient(...);                   // [TODO] Удалить клиента
```

#### Внутренние методы:
```cpp
void createServerSocket();    // Создать TCP сокет
void bindSocket();            // Привязать к порту
void listenSocket();          // Включить режим прослушивания
void setNonBlocking(int fd);  // Установить неблокирующий режим
int getServerFd() const;      // Получить fd главного сокета
```

### 4. Client & MessageBuffer

Эти классы содержат только заглушки на текущий момент. Будут заполнены в следующих этапах разработки.

## Основной цикл выполнения

```
main()
  ↓
Config::parseArgs(argc, argv)  // Парсинг аргументов
  ↓
Server::Server(config)         // Создание сервера
  ↓
Server::start()                // Инициализация сокета
  ├─ createServerSocket()
  ├─ bindSocket()
  ├─ listenSocket()
  ├─ new Poller(this)
  └─ poller_->addFd(serverSocketFd_, POLLIN)
  ↓
Server::run()                  // Основной цикл
  ├─ poller_->poll(timeout)
  └─ poller_->processEvents()  // Если готовые fd
       ├─ handleNewConnection()
       ├─ handleClientInput(fd)
       └─ disconnectClient(fd)
```

## Архитектурные решения

### 1. Forward Declaration в Poller.hpp

**Проблема:** Циклическая зависимость (Poller нужен Server*, Server нужен Poller*)

**Решение:** 
```cpp
// В Poller.hpp
class Server;  // Forward declaration, не подключаем Server.hpp

class Poller {
private:
    Server* server_;  // Можно использовать указатель
};
```

**Почему это работает:**
- Указатель имеет известный размер, не нужно знать полную структуру класса
- Полное определение Server подключается в Poller.cpp

### 2. Только poll() в Poller

**Почему это важно:**
- Централизация I/O мультиплексинга
- Легче отладка (одно место, где происходит блокировка)
- Следование командным соглашениям (TEAM_CONVENTIONS.md)

**Каждый класс отвечает за свое:**
- Poller → poll() и processEvents()
- Server → управление сокетами и клиентами
- Parser → парсинг сообщений (в будущем)
- CommandRegistry → выполнение команд (в будущем)

### 3. Неблокирующие сокеты

Все сокеты установлены в неблокирующий режим:
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

**Почему это нужно:**
- poll() работает с неблокирующими сокетами
- Если recv() вернет EAGAIN → данные не готовы, цикл продолжается
- Если recv() вернет 0 → соединение закрыто

## Что осталось сделать (TODO)

### Фаза 2: Управление клиентами
- [ ] Реализовать Server::handleNewConnection()
  - Accept новое соединение
  - Создать Client объект
  - Добавить fd в Poller с POLLIN
- [ ] Реализовать Client класс
  - Хранить nickame, username, состояние аутентификации
  - Содержать MessageBuffer

### Фаза 3: Обработка данных
- [ ] Реализовать Server::handleClientInput()
  - recv() с обработкой EAGAIN
  - Append в Client::messageBuffer_
  - extractMessages() для полных сообщений
- [ ] Реализовать MessageBuffer
  - append() для добавления данных
  - extractMessages() для извлечения полных \r\n сообщений

### Фаза 4: Команды и ответы
- [ ] Реализовать Server::sendToClient()
  - send() с обработкой ошибок
  - Логирование отправленных данных
- [ ] Подключить Parser и CommandRegistry
  - Парсинг IRC команд
  - Выполнение PASS, NICK, USER и т.д.

## Как компилировать и тестировать

### Сборка:
```bash
make clean
make
```

### Запуск:
```bash
./ircserv 6667 mypassword
```

Сервер должен вывести:
```
[Server] Created with port=6667
[Server] Bound to port 6667
[Server] Listening backlog=10
[Server] Listening on port 6667
[Server] Running event loop...
```

### Тестирование (пока):
```bash
# В другом терминале
nc localhost 6667
# Пока ничего не произойдет (handleNewConnection не реализована)
```

## Командные соглашения (из TEAM_CONVENTIONS.md)

### 1. Poll() ответственность
- **Только Poller** вызывает poll()
- Server НЕ вызывает poll()
- Другие классы НЕ вызывают poll()

### 2. Отправка ответов
- **Все** ответы идут через Server::sendToClient()
- Никогда не пишем прямо в fd
- Command handlers НЕ пишут напрямую в сокет

### 3. Взаимодействие слоев
```
Network Layer (Poller, Server)
        ↓ вызывает
Parser Layer (Parser, Replies)
        ↓ вызывает
Logic Layer (Client, Channel, Commands)
```

## Файлы, которые были изменены

### Создано:
- `src/Config.cpp` - реализация Config класса

### Раскомментировано в заголовках:
- `include/irc/Config.hpp` - декларации методов и полей
- `include/irc/Server.hpp` - декларации методов (Network слоя)
- `include/irc/Poller.hpp` - с forward declaration для Server

### Добавлено в реализации:
- `src/Server.cpp` - заглушки для методов, используемых Poller
- `src/main.cpp` - раскомментирован основной цикл запуска
- `src/Poller.cpp` - добавлен #include <cstring> для strerror()

### Удалено/Скрыто:
- Зависимость от Channel.hpp в Server
- Методы для управления каналами (пока закомментированы)

## Следующие шаги

1. Реализовать handleNewConnection() в Server
2. Создать Client класс с полной информацией
3. Реализовать MessageBuffer для буферизации данных
4. Добавить обработку handleClientInput() и disconnectClient()
5. Подключить слой парсера для обработки IRC команд

## Ссылки

- [TEAM_CONVENTIONS.md](TEAM_CONVENTIONS.md) - командные соглашения
- [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md) - полный план разработки
- [our_comments.md](our_comments.md) - детальные комментарии реализации

# Network Layer — как это работает

> Объяснение сетевого слоя ft_irc без изменений кода.
> Читай вместе с исходниками: [`src/Server.cpp`](../src/Server.cpp), [`src/Poller.cpp`](../src/Poller.cpp), [`include/irc/Server.hpp`](../include/irc/Server.hpp), [`include/irc/Poller.hpp`](../include/irc/Poller.hpp), [`include/irc/Client.hpp`](../include/irc/Client.hpp).

---

## Общая схема

```
main()
  └─ Server::start()   — создаёт socket, bind, listen
  └─ Server::run()     — главный цикл

       ┌─── Poller::poll() ───────────────┐
       │   (системный вызов poll())        │
       │   ждёт событий на всех fd         │
       └──────────────────────────────────┘
                     │
         ┌───────────┴────────────┐
    serverFd                  clientFd
    (POLLIN)                 (POLLIN/POLLHUP)
         │                        │
handleNewConnection()      handleClientInput()
   accept() → новый fd        recv() → данные
   Client + MessageBuffer    → MessageBuffer
   addFd(Poller)             → extractMessages()
                             → (Parser + Commands — TODO)
```

---

## Шаг за шагом

### 1. `Server::start()` — инициализация

Файл: [`src/Server.cpp`](../src/Server.cpp)

- `socket(AF_INET, SOCK_STREAM, 0)` — создаёт TCP-сокет
- `SO_REUSEADDR` — чтобы переиспользовать порт после рестарта
- `bind()` — привязывает к `0.0.0.0:<port>`
- `listen(backlog=10)` — ставит в очередь до 10 входящих соединений
- `setNonBlocking(serverFd)` — переводит в неблокирующий режим (`O_NONBLOCK`)
- `Poller::addFd(serverFd, POLLIN)` — регистрирует серверный сокет на отслеживание

---

### 2. `Server::run()` — главный цикл

Файл: [`src/Server.cpp`](../src/Server.cpp)

```cpp
while (running_) {
    poller_->poll(1000ms);    // блокируется до события (таймаут 1 сек)
    poller_->processEvents(); // обрабатывает готовые fd
}
```

`SIGINT` (Ctrl+C) выставляет `running_ = false` → цикл завершается чисто.

---

### 3. `Poller::poll()` — единственное место вызова `poll()`

Файл: [`src/Poller.cpp`](../src/Poller.cpp) | Заголовок: [`include/irc/Poller.hpp`](../include/irc/Poller.hpp)

- Хранит `std::vector<pollfd>` — список всех fd (server + все клиенты)
- Вызывает `::poll()` с таймаутом 1 секунда
- Возвращает количество готовых fd

> Конвенция команды: **только** `Poller` вызывает `poll()`. См. [`docs/TEAM_CONVENTIONS.md`](TEAM_CONVENTIONS.md).

---

### 4. `Poller::processEvents()` — диспетчер событий

Файл: [`src/Poller.cpp`](../src/Poller.cpp)

Для каждого fd с ненулевым `revents`:

| Условие | Действие |
|---|---|
| `fd == serverFd` + `POLLIN` | `server_->handleNewConnection()` |
| `clientFd` + `POLLIN` или `POLLHUP` | `server_->handleClientInput(fd)` |
| `clientFd` + `POLLERR` | `server_->disconnectClient(fd)` |

---

### 5. `handleNewConnection()` — принятие клиента

Файл: [`src/Server.cpp`](../src/Server.cpp)

```
accept()            → clientFd
setNonBlocking()    → O_NONBLOCK
new Client(fd)      → clients_[fd]
new MessageBuffer() → buffers_[fd]
poller_->addFd(fd, POLLIN)
```

Объект [`Client`](../include/irc/Client.hpp) хранит состояние регистрации, ник, username и т.д.  
Объект [`MessageBuffer`](../include/irc/MessageBuffer.hpp) накапливает сырые байты от `recv()`.

---

### 6. `handleClientInput(fd)` — чтение данных

Файл: [`src/Server.cpp`](../src/Server.cpp)

```
recv(fd, buf, 4095) → bytesRead
  bytesRead == 0  → disconnectClient()     (клиент закрыл соединение)
  bytesRead <  0  → EAGAIN: ждём дальше
                    иначе:  disconnectClient()
  bytesRead >  0  → msgBuffer->append(data)
                    msgBuffer->extractMessages()
                    → для каждого полного сообщения:
                        Parser::parse()         (TODO)
                        CommandRegistry::exec() (TODO)
```

`MessageBuffer` аккумулирует куски данных и отдаёт только **полные** IRC-сообщения (заканчивающиеся на `\r\n`). Подробнее: [`docs/BUFFER_ARCHITECTURE/`](BUFFER_ARCHITECTURE/).

---

### 7. `sendToClient(fd, msg)` — отправка данных клиенту

Файл: [`src/Server.cpp`](../src/Server.cpp)

- Добавляет `\r\n` если нет
- `send(fd, ...)` — неблокирующая запись в сокет
- При ошибке (не `EAGAIN`) → `disconnectClient(fd)`

> Это **единственный** метод для отправки данных клиентам (конвенция). Все команды вызывают именно его.

---

### 8. `disconnectClient(fd)` — отключение клиента

Файл: [`src/Server.cpp`](../src/Server.cpp)

1. Найти `Client*` в `clients_`
2. Удалить `MessageBuffer` из `buffers_`
3. `poller_->removeFd(fd)` — убрать из отслеживания
4. `close(fd)` — закрыть сокет
5. `delete client` — освободить память

---

## Ключевые принципы

| Принцип | Реализация |
|---|---|
| **Один поток** | `poll()` мультиплексирует все fd |
| **Non-blocking I/O** | `O_NONBLOCK` + обработка `EAGAIN` |
| **Единственный вызов `poll()`** | Только в [`Poller`](../include/irc/Poller.hpp) |
| **Разделение ответственности** | `Server` — логика, `Poller` — события, `Client` — состояние, `MessageBuffer` — буферизация |

---

## Связанные файлы

| Файл | Роль |
|---|---|
| [`src/main.cpp`](../src/main.cpp) | Точка входа: парсит аргументы, создаёт `Config` и `Server` |
| [`src/Server.cpp`](../src/Server.cpp) | Всё сетевое: socket, bind, accept, recv, send |
| [`src/Poller.cpp`](../src/Poller.cpp) | Единственный вызов `poll()`, диспетчер событий |
| [`src/MessageBuffer.cpp`](../src/MessageBuffer.cpp) | Буферизация сырых байт → полные IRC-сообщения |
| [`include/irc/Server.hpp`](../include/irc/Server.hpp) | Интерфейс Server |
| [`include/irc/Poller.hpp`](../include/irc/Poller.hpp) | Интерфейс Poller |
| [`include/irc/Client.hpp`](../include/irc/Client.hpp) | Состояние клиента |
| [`include/irc/MessageBuffer.hpp`](../include/irc/MessageBuffer.hpp) | Буфер сообщений |
| [`docs/TEAM_CONVENTIONS.md`](TEAM_CONVENTIONS.md) | Конвенции команды |
| [`docs/BUFFER_ARCHITECTURE/`](BUFFER_ARCHITECTURE/) | Архитектура буфера |

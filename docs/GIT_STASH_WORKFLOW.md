# Git Stash Workflow / Git Stash Рабочий процесс

---

## EN — English version

### 1. Diagnosis — What is going on before touching anything

```bash
# See full status: staged, unstaged, untracked
git status

# Show exact line-level diff of a modified file
git diff src/Server.cpp

# Check the last few commits for context
git log --oneline -5

# See what the remote has that you don't (dry-run fetch)
git fetch --dry-run
```

**What to look for:**
- `modified:` — file tracked by git and changed locally
- `Untracked files:` — new files git does not know about yet
- If the diff looks like AI-generated garbage → discard
- If it looks like a real fix → stash and preserve

---

### 2. The stash scenario: Working dir → stash → pull → pop → resolve

```bash
# ── STEP 1: Save your local changes into the stash ──────────────────────────
# -u  = include Untracked files (new files not yet added)
# -m  = human-readable label so you know what is inside later
git stash push -u -m "description: what and why"

# ── STEP 2: Verify the stash was created ────────────────────────────────────
# Shows a numbered list: stash@{0} is the most recent
git stash list

# Optional: inspect what is inside stash@{0} before pulling
git stash show -p stash@{0}

# ── STEP 3: Pull the team's changes ─────────────────────────────────────────
git pull

# ── STEP 4: Restore your stashed changes on top ─────────────────────────────
# pop = apply stash@{0} AND remove it from the stash stack
git stash pop

# ── STEP 5 (if conflict): Resolve and finalise ──────────────────────────────
# Git will report: "CONFLICT in src/Server.cpp"
# Open the file, look for conflict markers:
#   <<<<<<< Updated upstream
#   ...team code...
#   =======
#   ...your code...
#   >>>>>>> Stashed changes
# Edit the file to the correct final state, then:
git add src/Server.cpp       # mark as resolved
git stash drop               # manually remove the stash entry (pop already did this if no conflict)
```

---

### 3. Discard scenario — when changes really are AI garbage

```bash
# ── Discard changes to a tracked file (restores last committed version) ─────
git restore src/Server.cpp

# ── Preview which untracked files would be deleted (safe dry-run) ───────────
git clean -n

# ── Actually delete untracked files in docs/ ────────────────────────────────
git clean -f docs/

# ── Nuclear option: discard ALL unstaged changes AND untracked files ─────────
# git restore . && git clean -fd   ← use with caution
```

---

### 4. Cheat sheet

| Goal | Command |
|---|---|
| See what changed | `git status` / `git diff <file>` |
| Stash everything (incl. untracked) | `git stash push -u -m "label"` |
| List stashes | `git stash list` |
| Inspect a stash | `git stash show -p stash@{0}` |
| Apply + remove top stash | `git stash pop` |
| Apply without removing | `git stash apply stash@{0}` |
| Delete a stash entry | `git stash drop stash@{0}` |
| Wipe all stashes | `git stash clear` |
| Discard file changes | `git restore <file>` |
| Delete untracked files (preview) | `git clean -n` |
| Delete untracked files (execute) | `git clean -f` |

---
---

## RU — Русская версия

### 1. Диагностика — разобраться что происходит перед тем как что-то трогать

```bash
# Полный статус: staged (проиндексировано), unstaged (изменено), untracked (новые файлы)
git status

# Точный diff с построчными изменениями конкретного файла
git diff src/Server.cpp

# Последние несколько коммитов для контекста
git log --oneline -5

# Что есть на remote чего нет у тебя (без скачивания)
git fetch --dry-run
```

**На что смотреть:**
- `modified:` — файл отслеживается git и изменён локально
- `Untracked files:` — новые файлы, которые git ещё не знает
- Если diff выглядит как мусор от AI → выбрасываем
- Если выглядит как реальный фикс → stash и сохраняем

---

### 2. Сценарий stash: Рабочий каталог → stash → pull → pop → конфликты

```bash
# ── ШАГ 1: Спрятать локальные изменения в stash ─────────────────────────────
# -u  = включая Untracked файлы (новые, ещё не добавленные в git)
# -m  = человекочитаемая метка, чтобы потом понять что внутри
git stash push -u -m "описание: что и зачем"

# ── ШАГ 2: Убедиться что stash создан ───────────────────────────────────────
# Показывает пронумерованный список: stash@{0} — самый свежий
git stash list

# Опционально: посмотреть содержимое stash@{0} до пула
git stash show -p stash@{0}

# ── ШАГ 3: Подтянуть изменения команды ──────────────────────────────────────
git pull

# ── ШАГ 4: Вернуть твои изменения поверх ────────────────────────────────────
# pop = применить stash@{0} И удалить его из стека
git stash pop

# ── ШАГ 5 (если конфликт): Разрешить и завершить ────────────────────────────
# Git сообщит: "CONFLICT in src/Server.cpp"
# Открываешь файл, ищешь маркеры конфликта:
#   <<<<<<< Updated upstream
#   ...код команды...
#   =======
#   ...твой код...
#   >>>>>>> Stashed changes
# Редактируешь файл до правильного финального вида, затем:
git add src/Server.cpp       # помечаем как разрешённый
git stash drop               # вручную удаляем запись stash (pop уже сделал это если конфликта не было)
```

---

### 3. Сценарий удаления — когда изменения реально мусор от AI

```bash
# ── Отменить изменения в отслеживаемом файле (вернуть последнюю закомиченную версию) ──
git restore src/Server.cpp

# ── Посмотреть какие untracked файлы были бы удалены (dry-run, ничего не удаляет) ──────
git clean -n

# ── Удалить untracked файлы в папке docs/ ────────────────────────────────────
git clean -f docs/

# ── Ядерный вариант: выбросить ВСЕ unstaged изменения И untracked файлы ──────
# git restore . && git clean -fd   ← использовать осторожно
```

---

### 4. Шпаргалка

| Цель | Команда |
|---|---|
| Посмотреть что изменилось | `git status` / `git diff <файл>` |
| Спрятать всё (включая untracked) | `git stash push -u -m "метка"` |
| Список stash-ей | `git stash list` |
| Посмотреть содержимое stash | `git stash show -p stash@{0}` |
| Применить и удалить верхний stash | `git stash pop` |
| Применить без удаления | `git stash apply stash@{0}` |
| Удалить конкретный stash | `git stash drop stash@{0}` |
| Очистить все stash-и | `git stash clear` |
| Выбросить изменения в файле | `git restore <файл>` |
| Удалить untracked файлы (preview) | `git clean -n` |
| Удалить untracked файлы (выполнить) | `git clean -f` |

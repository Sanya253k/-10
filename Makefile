# =============================================================================
# Makefile - библиотека SparseVector
# =============================================================================
# Цели:
#   all        - собрать .so & приложение
#   shared     - собрать только разделяемую библиотеку
#   app        - собрать демонстрационное приложение
#   run        - запустить демонстрационное приложение
#   test-c     - собрать и запустить C-тесты
#   test-py    - запустить Python-тесты
#   test       - запустить все тесты
#   syntax     - проверить синтаксис исходников
#   analyze    - статический анализ
#   docs-html  - сформировать HTML-документацию Doxygen
#   docs-pdf   - сформировать PDF-документацию Doxygen
#   clean      - удалить все артефакты сборки
# =============================================================================

CC          := gcc
CFLAGS      := -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS     :=

# Каталоги
SRC_DIR     := src
INC_DIR     := include
APP_DIR     := app
TEST_DIR    := tests
BUILD_DIR   := build
REPORTS_DIR := reports

# Исходники библиотеки
LIB_SRCS    := $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS    := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))

# Артефакты
LIB_SO      := $(BUILD_DIR)/libsparse_vector.so
APP_BIN     := $(BUILD_DIR)/demo
TEST_BIN    := $(BUILD_DIR)/test_sparse_vector

# =============================================================================
# Цель по умолчанию
# =============================================================================

.PHONY: all
all: shared app

# =============================================================================
# Разделяемая библиотека
# =============================================================================

.PHONY: shared
shared: $(LIB_SO)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(REPORTS_DIR):
	mkdir -p $(REPORTS_DIR)

# Объектные файлы библиотеки
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -I$(INC_DIR) -c $< -o $@

$(LIB_SO): $(LIB_OBJS)
	$(CC) -shared -o $@ $^

# =============================================================================
# Демонстрационное приложение
# =============================================================================

.PHONY: app
app: $(APP_BIN)

$(APP_BIN): $(APP_DIR)/main.c $(LIB_SO)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< -o $@ -L$(BUILD_DIR) -lsparse_vector -Wl,-rpath,$(abspath $(BUILD_DIR))

.PHONY: run
run: $(APP_BIN)
	$(APP_BIN)

# =============================================================================
# C-тесты
# =============================================================================

$(TEST_BIN): $(TEST_DIR)/test_sparse_vector.c $(LIB_SO)
	$(CC) $(CFLAGS) -I$(INC_DIR) $< -o $@ -L$(BUILD_DIR) -lsparse_vector -lm -Wl,-rpath,$(abspath $(BUILD_DIR))

.PHONY: test-c
test-c: $(TEST_BIN)
	@echo "--- Запуск C-тестов ---"
	$(TEST_BIN)

# =============================================================================
# Python-тесты
# =============================================================================

.PHONY: test-py
test-py: $(LIB_SO)
	@echo "--- Запуск Python-тестов ---"
	python3 $(TEST_DIR)/test_sparse_vector.py

# =============================================================================
# Все тесты
# =============================================================================

.PHONY: test
test: test-c test-py

# =============================================================================
# Статический анализ
# =============================================================================

.PHONY: syntax
syntax:
	@echo "--- Проверка синтаксиса ---"
	$(CC) $(CFLAGS) -I$(INC_DIR) -fsyntax-only $(LIB_SRCS) $(APP_DIR)/main.c

.PHONY: analyze
analyze: $(REPORTS_DIR)
	@echo "--- gcc -fanalyzer ---"
	$(CC) $(CFLAGS) -fanalyzer -I$(INC_DIR) -c $(LIB_SRCS) -o /dev/null \
		2>&1 | tee $(REPORTS_DIR)/gcc_analyzer.txt || true
	@echo ""
	@echo "--- cppcheck ---"
	cppcheck --enable=all --inconclusive --std=c99 \
		--suppress=missingIncludeSystem \
        --suppress=duplicateCondition \
        --suppress=unusedFunction \
		-I$(INC_DIR) $(SRC_DIR) $(APP_DIR) $(TEST_DIR) \
		2>&1 | tee $(REPORTS_DIR)/cppcheck.txt || true
	@echo ""
	@echo "Отчёты сохранены в $(REPORTS_DIR)/"

# =============================================================================
# Документация Doxygen
# =============================================================================

.PHONY: docs-html
docs-html:
	@echo "--- Генерация HTML-документации ---"
	doxygen Doxyfile
	@echo "HTML: docs/html/index.html"

# =============================================================================
# Очистка
# =============================================================================

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(REPORTS_DIR) docs
	@echo "Очистка завершена."

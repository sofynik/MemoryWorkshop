#include <array>
#include <cctype>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct Question {
    std::string stage;
    std::string title;
    std::string text;
    std::string code;
    std::array<std::string, 3> answers;
    int correct;
    std::string explanation;
};

std::optional<int> readChoice(int minValue, int maxValue) {
    std::string line;

    while (std::getline(std::cin, line)) {
        std::size_t first = line.find_first_not_of(" \t\r\n");

        if (first != std::string::npos) {
            std::size_t last = line.find_last_not_of(" \t\r\n");

            if (first == last) {
                char c = line[first];

                if (c >= '0' + minValue &&
                    c <= '0' + maxValue) {
                    return c - '0';
                }
            }
        }

        std::cout << "Некорректный ввод. Введите число от "
                  << minValue << " до " << maxValue
                  << ": ";
    }

    return std::nullopt;
}

bool waitForEnter() {
    std::string line;

    while (true) {
        std::cout << "\nНажмите Enter для продолжения: ";

        if (!std::getline(std::cin, line)) {
            return false;
        }

        bool onlySpaces = true;

        for (unsigned char c : line) {
            if (!std::isspace(c)) {
                onlySpaces = false;
                break;
            }
        }

        if (onlySpaces) {
            return true;
        }

        std::cout << "Нужно нажать Enter без текста.\n";
    }
}

std::vector<Question> createQuestions() {
    return {
        {
            "Опасная копия",
            "Копирование указателя",
            "Выполняется RawArray b = a. Что делает "
            "неявный конструктор копирования с полем data?",
            R"(struct RawArray {
    int* data = new int[3]{10, 20, 30};
    ~RawArray() { delete[] data; }
};

RawArray a;
RawArray b = a;
b.data[0] = 99;)",
            {
                "Создаёт новый буфер и копирует в него элементы.",
                "Копирует значение указателя: оба объекта ссылаются на один буфер.",
                "Не компилируется, потому что объявлен деструктор."
            },
            2,
            "Неявный конструктор выполняет поэлементное копирование "
            "полей. Для int* копируется адрес, поэтому оба объекта "
            "ссылаются на один буфер, нарушая единоличное владение."
        },
        {
            "Опасная копия",
            "Общие данные",
            "Сразу после записи b.data[0] = 99, до разрушения "
            "объектов, какое значение хранится в a.data[0]?",
            R"(RawArray a;
RawArray b = a;
b.data[0] = 99;)",
            {
                "99, потому что оба указателя ведут к одному элементу.",
                "10, потому что b всегда имеет независимый буфер.",
                "Такая запись не компилируется."
            },
            1,
            "Оба указателя ссылаются на один буфер. Поэтому "
            "изменение первого элемента через b также видно "
            "через a, пока объекты ещё существуют."
        },
        {
            "Опасная копия",
            "Разрушение объектов",
            "Что неправильно при уничтожении a и b, "
            "если их указатели всё ещё содержат один адрес?",
            R"(~RawArray() {
    delete[] data;
})",
            {
                "Ничего: delete[] автоматически считает владельцев.",
                "Второй delete[] обязан бросить std::bad_alloc.",
                "Один буфер освобождается повторно, возникает undefined behavior."
            },
            3,
            "Оба деструктора пытаются освободить один буфер. "
            "Повторный delete[] приводит к неопределённому "
            "поведению, но не обязательно к падению."
        },
        {
            "Правильное владение",
            "Порядок присваивания",
            "В этом операторе копируются int. "
            "Какое утверждение верно?",
            R"(IntArray& operator=(const IntArray& other) {
    int* fresh = other.size
        ? new int[other.size] : nullptr;

    for (std::size_t i = 0; i < other.size; ++i)
        fresh[i] = other.data[i];

    delete[] data;
    data = fresh;
    size = other.size;
    return *this;
})",
            {
                "До new надо выполнить delete[] data, иначе старый буфер утечёт.",
                "Если new бросит, объект сохранится; a = a здесь также безопасно.",
                "Любое a = a вызывает double free, если нет отдельного if."
            },
            2,
            "Новый буфер сначала выделяется и заполняется. "
            "Если new бросит исключение, старое значение сохранится. "
            "Копирующий конструктор создаёт новый объект, а "
            "копирующее присваивание заменяет значение существующего."
        },
        {
            "Правильное владение",
            "Правила трёх, пяти и нуля",
            "Какой подход к специальным членам класса корректен?",
            "",
            {
                "Согласовать деструктор, copy и при необходимости move; "
                "при владении через RAII-поля предпочесть правило нуля.",
                "Деструктор автоматически запрещает copy и реализует move.",
                "Правило нуля запрещает классу владеть std::vector."
            },
            1,
            "Если класс самостоятельно владеет ресурсом, его операции "
            "копирования, перемещения и уничтожения должны быть "
            "согласованы. RAII-типы, например std::vector, позволяют "
            "часто следовать правилу нуля."
        },
        {
            "Передача ресурсов",
            "Смысл std::move",
            "Что делает само выражение std::move(a)?",
            R"(std::move(a);)",
            {
                "Немедленно обнуляет все поля a.",
                "Безусловно вызывает перемещающий конструктор.",
                "Даёт xvalue; изменение ресурса выполняет выбранная затем операция."
            },
            3,
            "std::move преобразует выражение в xvalue, позволяя "
            "выбрать подходящую перегрузку. Сам std::move не "
            "переносит ресурс и не вызывает конструктор."
        },
        {
            "Передача ресурсов",
            "Перемещающее присваивание",
            "Предположим, data — единолично владеющий int*, "
            "size — размер. Какой план подходит для "
            "operator=(IntArray&& other) с контрактом "
            "«self-move сохраняет значение»?",
            R"(IntArray& operator=(IntArray&& other) noexcept {
})",
            {
                "Если this == &other, вернуть *this. Иначе "
                "освободить свой буфер, принять поля other, "
                "обнулить other и вернуть *this.",
                "Скопировать поля other, оставив источник без изменений, "
                "чтобы оба объекта владели буфером.",
                "Вызвать delete[] other.data, затем скопировать "
                "этот адрес в data."
            },
            1,
            "Проверка this == &other сохраняет значение при "
            "самоперемещении. Иначе ресурс передаётся новому "
            "владельцу, а указатель источника обнуляется, "
            "предотвращая двойное освобождение."
        },
        {
            "Передача ресурсов",
            "Состояние источника",
            "Контракт учебного IntArray: после перемещения "
            "в другой объект источник имеет size == 0 и "
            "data == nullptr. Какие действия допустимы?",
            "",
            {
                "Объект уже уничтожен; использовать его нельзя никак.",
                "Прочитать size, проверить data, присвоить новое "
                "значение или уничтожить объект; a[0] читать нельзя.",
                "Прочитать a[0]: прежний элемент обязательно "
                "сохранился в источнике."
            },
            2,
            "После перемещения источник остаётся живым объектом. "
            "В нашем IntArray он пуст, поэтому индексировать его "
            "нельзя: не выполняется предусловие индексации. "
            "Другие типы могут иметь иной контракт."
        },
        {
            "Операторы под контролем",
            "Сравнение содержимого",
            "a и b независимо владеют буферами {10, 20, 30}. "
            "Их адреса различны. operator== сравнивает размер "
            "и элементы. Чему равно a == b?",
            R"(bool operator==(const IntArray& other) const {
    if (size != other.size)
        return false;

    for (std::size_t i = 0; i < size; ++i)
        if (data[i] != other.data[i])
            return false;

    return true;
})",
            {
                "false, потому что адреса различаются.",
                "Выражение нельзя вычислить для двух объектов.",
                "true, потому что размеры и значения элементов совпадают."
            },
            3,
            "operator== сравнивает содержимое массивов: "
            "размеры и значения элементов. Разные адреса "
            "буферов не мешают объектам быть равными."
        },
        {
            "Операторы под контролем",
            "Константная индексация",
            "Есть две перегрузки ниже. a — обычный массив, "
            "c — const-массив, оба размера 3. "
            "Какое утверждение верно?",
            R"(int& operator[](std::size_t i) {
    return data[i];
}

const int& operator[](std::size_t i) const {
    return data[i];
})",
            {
                "c[0] = 5 допустимо, потому что метод const.",
                "a[0] = 5 и int x = c[0] допустимы; "
                "c[0] = 5 не компилируется.",
                "a[10] = 5 обязательно бросит исключение."
            },
            2,
            "Неконстантная перегрузка возвращает int&, "
            "разрешая запись. Константная возвращает const int&, "
            "поэтому запись запрещена. Индексация требует "
            "допустимого индекса; operator[] здесь границы не проверяет."
        }
    };
}

void showMenu() {
    std::cout << "\n=============================\n";
    std::cout << "     МАСТЕРСКАЯ ПАМЯТИ\n";
    std::cout << "=============================\n";
    std::cout << "1. Начать новую игру\n";
    std::cout << "2. Правила\n";
    std::cout << "0. Выход\n";
    std::cout << "Ваш выбор: ";
}

void showRules() {
    std::cout << "\n=== ПРАВИЛА ===\n";
    std::cout << "В игре 4 этапа и 10 вопросов.\n";
    std::cout << "На каждый вопрос есть 3 варианта ответа.\n";
    std::cout << "Выберите один вариант: 1, 2 или 3.\n";
    std::cout << "За правильный ответ: 10 баллов.\n";
    std::cout << "За неправильный ответ: 0 баллов.\n";
    std::cout << "Максимум: 100 баллов.\n";
    std::cout << "После ответа показывается объяснение.\n";
    std::cout << "После объяснения нажмите Enter.\n";
    std::cout << "Новая игра сбрасывает результат.\n";
    std::cout << "\nОценки:\n";
    std::cout << "90-100: Отличное понимание темы\n";
    std::cout << "70-89: Уверенное понимание материала\n";
    std::cout << "50-69: Базовые знания, рекомендуется повторение\n";
    std::cout << "0-49: Стоит повторить материал\n";
}

void showQuestion(const Question& q, int number) {
    std::cout << "\n====================================\n";
    std::cout << "Этап: " << q.stage << '\n';
    std::cout << "Вопрос " << number << "/10\n";
    std::cout << q.title << '\n';
    std::cout << "====================================\n";

    std::cout << q.text << "\n";

    if (!q.code.empty()) {
        std::cout << "\nФрагмент кода:\n";
        std::cout << q.code << '\n';
    }

    std::cout << "\nВарианты ответа:\n";

    for (std::size_t i = 0; i < q.answers.size(); ++i) {
        std::cout << i + 1 << ") " << q.answers[i] << '\n';
    }

    std::cout << "\nВаш ответ (1-3): ";
}

void showResult(int correctCount) {
    int score = correctCount * 10;

    std::cout << "\n========== РЕЗУЛЬТАТ ==========\n";
    std::cout << "Правильно: " << correctCount << " из 10\n";
    std::cout << "Баллы: " << score << " из 100\n";

    if (score >= 90) {
        std::cout << "Отличное понимание темы\n";
    } else if (score >= 70) {
        std::cout << "Уверенное понимание материала\n";
    } else if (score >= 50) {
        std::cout << "Базовые знания, рекомендуется повторение\n";
    } else {
        std::cout << "Стоит повторить материал\n";
    }
}

bool askQuestion(const Question& q, int number, int& correctCount) {
    showQuestion(q, number);

    std::optional<int> answer = readChoice(1, 3);

    if (!answer) {
        return false;
    }

    if (*answer == q.correct) {
        std::cout << "\nВерно!\n";
        ++correctCount;
    } else {
        std::cout << "\nНеверно!\n";
    }

    std::cout << "Правильный ответ: "
              << q.correct << ") "
              << q.answers[q.correct - 1] << '\n';

    std::cout << "Пояснение: "
              << q.explanation << '\n';

    return waitForEnter();
}

bool playGame(const std::vector<Question>& questions) {
    int correctCount = 0;

    std::cout << "\nНачинается новая игра!\n";

    for (std::size_t i = 0; i < questions.size(); ++i) {
        if (!askQuestion(questions[i],
                         static_cast<int>(i + 1),
                         correctCount)) {
            std::cout << "\nИгра прервана: конец ввода.\n";
            std::cout << "Итоговая оценка не присваивается.\n";
            return false;
        }
    }

    showResult(correctCount);
    return true;
}

int main() {
    const std::vector<Question> questions = createQuestions();

    while (true) {
        showMenu();

        std::optional<int> choice = readChoice(0, 2);

        if (!choice) {
            std::cout << "\nЗавершение: конец ввода.\n";
            break;
        }

        if (*choice == 0) {
            std::cout << "\nДо свидания!\n";
            break;
        }

        if (*choice == 2) {
            showRules();
            continue;
        }

        if (*choice == 1) {
            if (!playGame(questions)) {
                break;
            }
        }
    }

    return 0;
}
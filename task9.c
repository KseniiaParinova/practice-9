#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BOOKS 1000
#define MAX_READERS 500
#define MAX_LOANS 2000

// --- Структуры данных (из вашего описания) ---

typedef struct {
    int id;
    char title[200];
    char author[100];
    char isbn[20];
    int year;
    char genre[50];
    int total_copies;
    int available_copies;
} Book;

typedef struct {
    int id;
    char name[100];
    char email[100];
    char phone[20];
    time_t registered_at;
} Reader;

typedef struct {
    int id;
    int book_id;
    int reader_id;
    time_t borrowed_at;
    time_t due_date;
    time_t returned_at; // 0 если не возвращена
} Loan;

// --- Глобальные массивы данных (In-Memory БД) ---
Book catalog[MAX_BOOKS];
int book_count = 0;

Reader readers[MAX_READERS];
int reader_count = 0;

Loan history[MAX_LOANS];
int loan_count = 0;

int next_book_id = 1;
int next_reader_id = 1;
int next_loan_id = 1;

// --- Вспомогательные функции ---

void clear_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- Функциональность ---

// 1. Добавление книги
void add_book() {
    if (book_count >= MAX_BOOKS) {
        printf("Каталог заполнен!\n");
        return;
    }
    Book b;
    b.id = next_book_id++;
    
    printf("Введите название: ");
    fgets(b.title, 200, stdin);
    b.title[strcspn(b.title, "\n")] = 0;
    
    printf("Введите автора: ");
    fgets(b.author, 100, stdin);
    b.author[strcspn(b.author, "\n")] = 0;
    
    printf("Введите ISBN: ");
    scanf("%19s", b.isbn);
    
    printf("Введите год издания: ");
    scanf("%d", &b.year);
    
    clear_buffer(); // Очистка буфера после scanf
    printf("Введите жанр: ");
    fgets(b.genre, 50, stdin);
    b.genre[strcspn(b.genre, "\n")] = 0;
    
    printf("Количество копий: ");
    scanf("%d", &b.total_copies);
    b.available_copies = b.total_copies;
    clear_buffer();

    catalog[book_count++] = b;
    printf("Книга успешно добавлена! ID: %d\n", b.id);
}

// 2. Поиск книги по автору, названию или ISBN
void search_books() {
    char query[100];
    printf("Введите строку для поиска (автор, название или ISBN): ");
    fgets(query, 100, stdin);
    query[strcspn(query, "\n")] = 0;

    printf("\n--- Результаты поиска ---\n");
    bool found = false;
    for (int i = 0; i < book_count; i++) {
        if (strstr(catalog[i].title, query) != NULL ||
            strstr(catalog[i].author, query) != NULL ||
            strstr(catalog[i].isbn, query) != NULL) {
            
            printf("ID: %d | '%s' (Автор: %s) | ISBN: %s | Доступно: %d/%d\n", 
                   catalog[i].id, catalog[i].title, catalog[i].author, 
                   catalog[i].isbn, catalog[i].available_copies, catalog[i].total_copies);
            found = true;
        }
    }
    if (!found) printf("Книги не найдены.\n");
}

// 3. Регистрация читателя
void add_reader() {
    if (reader_count >= MAX_READERS) return;
    Reader r;
    r.id = next_reader_id++;
    
    printf("ФИО читателя: ");
    fgets(r.name, 100, stdin);
    r.name[strcspn(r.name, "\n")] = 0;
    
    printf("Email: ");
    scanf("%99s", r.email);
    printf("Телефон: ");
    scanf("%19s", r.phone);
    clear_buffer();
    
    r.registered_at = time(NULL);
    readers[reader_count++] = r;
    printf("Читатель зарегистрирован! ID: %d\n", r.id);
}

// 4. Выдача книги
void issue_book() {
    int b_id, r_id;
    printf("Введите ID книги: ");
    scanf("%d", &b_id);
    printf("Введите ID читателя: ");
    scanf("%d", &r_id);
    clear_buffer();

    // Проверка книги
    int b_index = -1;
    for (int i = 0; i < book_count; i++) {
        if (catalog[i].id == b_id) { b_index = i; break; }
    }
    
    if (b_index == -1 || catalog[b_index].available_copies <= 0) {
        printf("Книга не найдена или нет доступных экземпляров.\n");
        return;
    }

    // Оформление выдачи
    Loan l;
    l.id = next_loan_id++;
    l.book_id = b_id;
    l.reader_id = r_id;
    l.borrowed_at = time(NULL);
    l.due_date = l.borrowed_at + (14 * 24 * 60 * 60); // Выдача на 14 дней
    l.returned_at = 0;

    history[loan_count++] = l;
    catalog[b_index].available_copies--;
    
    printf("Книга успешно выдана! Операция ID: %d\n", l.id);
}

// 5. Возврат книги
void return_book() {
    int b_id, r_id;
    printf("Введите ID книги: ");
    scanf("%d", &b_id);
    printf("Введите ID читателя: ");
    scanf("%d", &r_id);
    clear_buffer();

    for (int i = 0; i < loan_count; i++) {
        if (history[i].book_id == b_id && history[i].reader_id == r_id && history[i].returned_at == 0) {
            history[i].returned_at = time(NULL);
            
            // Увеличиваем количество доступных книг
            for (int j = 0; j < book_count; j++) {
                if (catalog[j].id == b_id) {
                    catalog[j].available_copies++;
                    break;
                }
            }
            printf("Книга возвращена! Спасибо.\n");
            return;
        }
    }
    printf("Активная выдача для этой книги и читателя не найдена.\n");
}

// 6. Отчёт о задолжниках
void report_overdue() {
    time_t now = time(NULL);
    printf("\n--- Задолжники ---\n");
    bool found = false;
    for (int i = 0; i < loan_count; i++) {
        if (history[i].returned_at == 0 && now > history[i].due_date) {
            printf("ID Операции: %d | ID Читателя: %d | ID Книги: %d | Просрочено!\n", 
                   history[i].id, history[i].reader_id, history[i].book_id);
            found = true;
        }
    }
    if (!found) printf("Задолжников нет.\n");
}

// 7. Статистика популярности книг
void report_popularity() {
    printf("\n--- Популярность книг (Количество выдач) ---\n");
    for (int i = 0; i < book_count; i++) {
        int count = 0;
        for (int j = 0; j < loan_count; j++) {
            if (history[j].book_id == catalog[i].id) {
                count++;
            }
        }
        if (count > 0) {
            printf("Книга: '%s' | Выдана: %d раз(а)\n", catalog[i].title, count);
        }
    }
}

// --- Главное меню ---
int main() {
    int choice;
    do {
        printf("\n=============================\n");
        printf(" СИСТЕМА УЧЕТА БИБЛИОТЕКИ\n");
        printf("=============================\n");
        printf("1. Добавить книгу\n");
        printf("2. Найти книгу\n");
        printf("3. Зарегистрировать читателя\n");
        printf("4. Выдать книгу\n");
        printf("5. Вернуть книгу\n");
        printf("6. Отчет о задолжниках\n");
        printf("7. Статистика популярности\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_buffer();
            continue;
        }
        clear_buffer();

        switch(choice) {
            case 1: add_book(); break;
            case 2: search_books(); break;
            case 3: add_reader(); break;
            case 4: issue_book(); break;
            case 5: return_book(); break;
            case 6: report_overdue(); break;
            case 7: report_popularity(); break;
            case 0: printf("Завершение работы...\n"); break;
            default: printf("Неверный выбор!\n");
        }
    } while (choice != 0);

    return 0;
}
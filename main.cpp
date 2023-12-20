#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <random>
#include <unistd.h>
#include <fstream>
#include <queue>

pthread_mutex_t rooms[30];   // Номера в отеле
int lastId = 0;             // id последнего посетителя
bool taken[30]{false}; // Записывает какие номера заняты

// структура посетителя
struct Visitor
{
    int id = 0; // id посетителя
    int days = 1; // количество суток в отеле
    
    Visitor(int id, int days = 1) // конструктор посетителя days по дефолту 1
    {
        this->id = id;
        this->days = days;
        lastId = id; // записываем в lastId id посетителя
    }
};

int work_time;        // Время работы отеля
bool is_working = true;        // Работает ли отель
pthread_mutex_t hotel_work; // Ресурс отеля

// Поток отеля
void *hotel(void *args)
{
    pthread_mutex_lock(&hotel_work); // Блокирует ресурс отеля
    sleep(work_time); // ждет время работы отеля
    is_working = false;
    pthread_mutex_unlock(&hotel_work); // Разблокирует

    return NULL;
}

// создаем рандомное количество суток от 1 до 3
int get_random_days()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 3);
    return dist(rng);
}

// Проверка на наличие свободного номера определенного типа
bool Check(int &room)
{
    bool res = false; // создаем флаг
    for (int i = 0; i < 30; ++i) // проходимся по всем комнатам
    {
        if (!taken[i]) // если свободна
        {
            res = true; // передаем флагу значение true
            room = i + 1; // переопределяем номер комнаты
            return res;
        }
    }
    return res;
}

// занимает комнату под номером
void take_room(int room_num)
{
    taken[room_num - 1] = true; // занимает комнату
}

std::queue<Visitor> queue_; // очередь

// Поток посетителя
void *guest(void *args)
{
    Visitor visitor = Visitor(++lastId, get_random_days()); //создаём посетителя со случайным количеством дней
    std::cout << "A visitor with id " << visitor.id << " wants to stay on " << visitor.days << " days"
              << "\n";

    int room_num = 0; // Номер комнаты
    queue_.push(visitor); // Добавляем его в очередь
    if (!(Check(room_num) && queue_.front().id == visitor.id)) // Если очередь была до него или нет свободных номеров, то сообщаем об этом
    {
        std::cout << "Visitor with id " << visitor.id << " couldn't take a room and wait in the queue"
                  << "\n";
    }
    bool take = false; // создаем флаг
    do
    {
        if (queue_.front().id == visitor.id && Check(room_num))
        { // Если первый в очереди и есть свободный номер
            queue_.pop(); // убираем из очереди
            take_room(room_num); // занимает комнату
            take = true; // делаем флаг true
            pthread_mutex_lock(&rooms[room_num]); // Блокирует ресурс комнаты
            std::cout << "Visitor with id " << visitor.id << " has taken room number " << room_num << " on " << visitor.days << " days \n";
            sleep(24 * visitor.days); // Занимает на 24 * (на количество суток) секунд
            taken[room_num - 1] = false; // Освобождает комнату
            std::cout << "Visitor with id " << visitor.id << " has left room number " << room_num << "\n";
            pthread_mutex_unlock(&rooms[room_num]); //Разблокируем комнату
        }
        else
        {
            sleep(1); // если нет, то ждёт секунду и пробует снова
        }
    } while (!take);

    return NULL;
}

int main(int argc, char *argv[])
{ // Входные значения с командной строки
    for (int i = 0; i < 30; ++i)
    {
        pthread_mutex_init(&rooms[i], NULL); // Инизиализация номеров отеля
    }

    pthread_mutex_init(&hotel_work, NULL); // Инициализация отеля

    work_time = std::stoi(argv[1]); // переводим считанное значение в int - это время работы отеля в секундах
    

    pthread_t hotel_ = nullptr;
    int count = 0; // счетчик посетителей

    while (is_working) // пока отель работает
    {
        pthread_create(&hotel_, NULL, hotel, NULL); // создаем поток отеля
        pthread_create(&hotel_, NULL, guest, NULL); // создаем поток гостя
        ++count;
        sleep(1); // ждем секунду
    }
    
    for (int i = 0; i < count; ++i) {
        pthread_join(hotel_, NULL);
    }
    std::cout << "\n----The hotel stop----";
}

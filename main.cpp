#include <cstdlib>
#include <ctime>
#include <thread>
#include <future>
#include <mutex>
#include <iostream>

bool make_thread = true;
int count_threads = 6;
std::mutex lock_count;

void merge(int* array, int l_index, int m_index, int r_index)
{
    // создаем временные массивы
    int l_size = m_index - l_index + 1;
    int r_size = r_index - m_index;
    int* l_array = new int[l_size];
    int* r_array = new int[r_size];

    // копируем данные во временные массивы
    for (int i = 0; i < l_size; i++)
    {
        l_array[i] = array[l_index + i];
    }
    for (int i = 0; i < r_size; i++)
    {
        r_array[i] = array[m_index + 1 + i];
    }

    int i = 0;
    int j = 0;
    int k = l_index; // начало левой части

    // записываем минимальные элементы обратно во входной массив
    while (i < l_size && j < r_size)
    {
        
        if (l_array[i] <= r_array[j])
        {
            array[k] = l_array[i];
            i++;
        }
        else
        {
            array[k] = r_array[j];
            j++;
        }
        k++;
    }

    // записываем оставшиеся элементы левой части
    while (i < l_size)
    {
        array[k] = l_array[i];
        i++;
        k++;
    }

    // записываем оставшиеся элементы правой части
    while (j < r_size)
    {
        array[k] = r_array[j];
        j++;
        k++;
    }
    delete[] l_array;
    delete[] r_array;
}


void mergeSort(int* array, int l_index, int r_index)
{
    if (l_index >= r_index)
    {
        return;
    }
    int m_index = (l_index + r_index) / 2;
    int count = 0;

    lock_count.lock();
    if (count_threads > 1)
    {
        count = --count_threads;
    }
    lock_count.unlock();

    if (make_thread && count > 0)
    {
        // если количество выделенных потоков меньше заданного count_threads
        // вызываем асинхронно рекурсию для одной части
        auto f = std::async(std::launch::async, [&]() {
            mergeSort(array, l_index, m_index); });
        mergeSort(array, m_index + 1, r_index);
    }
    else // запускаем обе части синхронно
    {
        mergeSort(array, l_index, m_index);
        mergeSort(array, m_index + 1, r_index);
    }
    merge(array, l_index, m_index, r_index);
}


int main()
{
    srand(0);
    int size = 1000000;
    int* array = new int[size];
    int* array_copy = new int[size];

    for (long i = 0; i < size; i++)
    {
        array[i] = rand() % 1000000;
        array_copy[i] = array[i];
    }

    // многопоточный запуск
    auto start = clock();
    mergeSort(array, 0, size - 1);
    auto end = clock();
    auto time = difftime(end, start);
    std::cout << "6 threads, time: " << time << std::endl;

    for (int i = 0; i < size - 1; i++)
    {
        if (array[i] > array[i + 1])
        {
            std::cout << "Unsorted" << std::endl;
            break;
        }
    }
    delete[] array;

    // однопоточный запуск
    make_thread = false;

    start = clock();
    mergeSort(array_copy, 0, size - 1);
    end = clock();
    time = difftime(end, start);
    std::cout << "1 thread, time: " << time << std::endl;

    for (int i = 0; i < size - 1; i++)
    {
        if (array_copy[i] > array_copy[i + 1])
        {
            std::cout << "Unsorted" << std::endl;
            break;
        }
    }
    delete[] array_copy;

    return 0;
}
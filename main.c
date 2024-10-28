#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <locale.h>

// Функция для интегрирования
double f(double x) {
    // Пример функции: f(x) = x sin x + 2
    //return fabs(x * sin(x));
    return fabs(exp(x) * sin(x * x));
}

// Метод трапеций для численного интегрирования
double trapezoidal_rule(double a, double b, double n) {
    double h = (b - a) / n;
    double sum = 0.5 * (f(a) + f(b));
    for (int i = 1; i < n; i++)
        sum += f(a + i * h);
    return sum * h;
}

int main(int argc, char* argv[]) {
    MPI_Status status;                                   // Чтобы узнать какой процесс и с каким тэгом посылает сообщение
    char* locale = setlocale(LC_ALL, "Ru");
    int proccess_num, cnt_process, n = 100000000;        // Номер процесса, количество процессов, количество интервалов
    int local_n;                                         // Количество обрабатываемых интервалов для одного процесса
    double a = 0.0, b = 3.0, h, local_a, local_b;
    double value = 0.0;
    double time1, time2, diff;
    double remainder;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proccess_num);  // Определяем номер текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &cnt_process);  // Определяем количество процессов
    h = (b - a) / n;                   // Интервал для трапеции
    local_n = n / cnt_process;         // Количество обрабатываемых интервалов для одного процесса
    remainder = n % cnt_process;       // Остаток, который обработается главным процессом

    // Если у нас первый процесс
    if (proccess_num == 0) {
        time1 = MPI_Wtime();                         // Засекаем время
        for (int i = 1; i < cnt_process; i++) {        // Проходимся по остальным процессам (для посылки данных)
            double interval[2] = { a + i * local_n * h, a + (i + 1) * local_n * h };   // Определяем границы интервала для процесса i
            MPI_Send(&interval, 2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
        value += trapezoidal_rule(a, a + local_n * h, local_n);                      // Вычисляем часть интеграла для главного процесса
        if (remainder != 0) {                                              // Если количество отрезков в остатке не нулевое
            local_a = a + cnt_process * local_n * h;                         // Находим левую границу для оставшегося интервала
            value += trapezoidal_rule(local_a, b, remainder);            // Находим оставшуюся часть интеграла
        }
        for (int i = 1; i < cnt_process; i++) {
            double s;                               // Часть интеграла, вычисленная процессом i
            MPI_Recv(&s, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            value += s;
        }
        time2 = MPI_Wtime();                        // Останавливаем таймер
        diff = time2 - time1;                       // Вычисляем разницу во времени
        printf("С n = %d трапециями, значение интеграла от %.2f до %.2f = %.15f\n", n, a, b, value);
        printf("Время вычисления t = %.8f сек\n", diff);
    }
    else {
        double interval[2];
        MPI_Recv(&interval, 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);    // Принимаем интервал от первого процесса
        double res = trapezoidal_rule(interval[0], interval[1], local_n);     // Вычисляем выделенную часть интеграла
        MPI_Send(&res, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);                  // Отсылаем вычисленное значение главному процессу
    }


    MPI_Finalize();
    return 0;
}

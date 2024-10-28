#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <locale.h>

// ������� ��� ��������������
double f(double x) {
    // ������ �������: f(x) = x sin x + 2
    //return fabs(x * sin(x));
    return fabs(exp(x) * sin(x * x));
}

// ����� �������� ��� ���������� ��������������
double trapezoidal_rule(double a, double b, double n) {
    double h = (b - a) / n;
    double sum = 0.5 * (f(a) + f(b));
    for (int i = 1; i < n; i++)
        sum += f(a + i * h);
    return sum * h;
}

int main(int argc, char* argv[]) {
    MPI_Status status;                                   // ����� ������ ����� ������� � � ����� ����� �������� ���������
    char* locale = setlocale(LC_ALL, "Ru");
    int proccess_num, cnt_process, n = 100000000;        // ����� ��������, ���������� ���������, ���������� ����������
    int local_n;                                         // ���������� �������������� ���������� ��� ������ ��������
    double a = 0.0, b = 3.0, h, local_a, local_b;
    double value = 0.0;
    double time1, time2, diff;
    double remainder;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proccess_num);  // ���������� ����� �������� ��������
    MPI_Comm_size(MPI_COMM_WORLD, &cnt_process);  // ���������� ���������� ���������
    h = (b - a) / n;                   // �������� ��� ��������
    local_n = n / cnt_process;         // ���������� �������������� ���������� ��� ������ ��������
    remainder = n % cnt_process;       // �������, ������� ������������ ������� ���������

    // ���� � ��� ������ �������
    if (proccess_num == 0) {
        time1 = MPI_Wtime();                         // �������� �����
        for (int i = 1; i < cnt_process; i++) {        // ���������� �� ��������� ��������� (��� ������� ������)
            double interval[2] = { a + i * local_n * h, a + (i + 1) * local_n * h };   // ���������� ������� ��������� ��� �������� i
            MPI_Send(&interval, 2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
        value += trapezoidal_rule(a, a + local_n * h, local_n);                      // ��������� ����� ��������� ��� �������� ��������
        if (remainder != 0) {                                              // ���� ���������� �������� � ������� �� �������
            local_a = a + cnt_process * local_n * h;                         // ������� ����� ������� ��� ����������� ���������
            value += trapezoidal_rule(local_a, b, remainder);            // ������� ���������� ����� ���������
        }
        for (int i = 1; i < cnt_process; i++) {
            double s;                               // ����� ���������, ����������� ��������� i
            MPI_Recv(&s, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            value += s;
        }
        time2 = MPI_Wtime();                        // ������������� ������
        diff = time2 - time1;                       // ��������� ������� �� �������
        printf("� n = %d ����������, �������� ��������� �� %.2f �� %.2f = %.15f\n", n, a, b, value);
        printf("����� ���������� t = %.8f ���\n", diff);
    }
    else {
        double interval[2];
        MPI_Recv(&interval, 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);    // ��������� �������� �� ������� ��������
        double res = trapezoidal_rule(interval[0], interval[1], local_n);     // ��������� ���������� ����� ���������
        MPI_Send(&res, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);                  // �������� ����������� �������� �������� ��������
    }


    MPI_Finalize();
    return 0;
}

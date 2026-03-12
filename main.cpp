#include <iostream>

using namespace std;
int main() {
  
    int b;
     b=60; 
    int a;
    cin >> a;


    int minuto; 
    minuto = 0;
    minuto = a / b;
    int segundo;
    segundo = 0;
    segundo = a % b;
    int hora;
    hora = 0;
    hora = minuto / b;
    int minuto_final;
    minuto_final = minuto - (hora * b);

    cout << hora << ":" <<  minuto_final << ":" << segundo << "\n";

    return 0;
}

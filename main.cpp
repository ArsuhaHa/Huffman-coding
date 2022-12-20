#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

struct Uzel
{
    int number; //кол-во встречающихся символов
    char symbol; //сам символ
    Uzel *left; //указатель на левого потомка
    Uzel *right; // указатель на правого потомка



    /*
     * конструктор, который создаёт родителя для его потомков
     * firts - первый потомок
     * second - второй потомок
     * его number - сумма number его потомков
     */
    Uzel(Uzel *first, Uzel *second)
    {
        left = first;
        right = second;
        number = first->number + second->number;
    }
    Uzel() {
        left = right = NULL;
    }
};

// структура для сортировки узлов
struct MyCompare
{
    // левый потомок меньше правого
    bool operator () (const Uzel *l, const Uzel *r) const
    {
        return l->number < r -> number;
    }

};

/*
 * Ниже представлено заполнение таблицы значений
 * Т.е., нам нужно, проходясь по дереву, выписывать их коды для символов
 * Для этого создам рекурсивную функцию BuildTable
 *
 */

vector<bool> code; //код символа
map <char, vector<bool>> table; //ассоциация символа с кодом

void BuildTable(Uzel *root)
{
    if (root-> left != NULL)  //если левая ветвь не пустая
    {
        code.push_back(0); //к коду добавляю 0
        BuildTable(root->left); //снова проверяю на левую ветвь
    }
    if (root -> right != NULL) //если правая ветвь не пустая
    {
        code.push_back(1); // коду добавляю 1
        BuildTable(root -> right); //снова проверяю на правую ветвь
    }
    if (root->left==NULL && root->right==NULL) //если дошли до символа
    {
        table[root -> symbol] = code; //ассоциирую код с самим символом
    }
    // чтобы вернуться на одну ветвь назад, удаляю последний элемент
    code.pop_back();
}


int main()
{
    //string s = "HHHello, Worrrld!"; //строка для кодирования
    ifstream file("input.txt");
    /*
     * циклом прохожусь по строке и при помощи ассоциативного массива
     * считаю колличество встречающихся символов в строке
     */
    map<char, int> m;

    while(!file.eof())
    {
        char c;
        c = file.get(); //считываем байт
        m[c]++;
    }

    /*
     * далее, будем создавать бинарное дерево
     * Каждый узел - это символ и его кол-во в строке
     * Для этого создам структуру Uzel
     */

    // для начала заполним list первоначальными узлами
    list<Uzel *> l; //создаю list

    /*
     * чтобы заполнить list символами, прохожусь по map
     * Каждый раз создавая указатель p структуры Uzel,
     * в поле symbol присваюваю поле first (символ из map)
     * в поле number присваюваю поле second (кол-во встреч символа из map)
     * заношу указатель p в list.
     * Таким образом, каждый различный символ строки станет узлом.
     */

    map<char, int>::iterator itr;
    for(itr = m.begin(); itr != m.end(); ++itr)
    {
        Uzel *p = new Uzel;
        p->symbol = itr->first;
        p->number = itr->second;
        l.push_back(p);
    }

    /*
     * далее, нужно сделать дерево
     * каждый раз буду сортировать список (при помощи MyCompare)
     * Таким образом, узлы с наименьшем number будут в начале списка
     * Потомки родителя будут первые два элемента list
     * Причём, number у родителя будет равен сумме number их потомков
     * Буду удалять первые два элемента списка, а их место займёт родитель
     */

    while (l.size() != 1)
    {
        l.sort(MyCompare());
        Uzel *first_descendant = l.front(); //адрес первого потомка
        l.pop_front(); //удаляю первого потомка
        Uzel *second_descendant = l.front(); //адрес второго потомка
        l.pop_front();

        //создаю родителя и заношу его в список
        Uzel *parent = new Uzel(first_descendant, second_descendant);
        l.push_back(parent);
    }

    /*
     * Так как удаляю по два элемента,
     * последним будет корень дерева
     * также занесём его в список
     */

    Uzel *root = l.front();

    /*
     * Таким образом получим дерево, у которого листья - символы строки
     */

    //Далее, функция для создания таблицы, где каждому символу соответствует его код
    BuildTable(root);

    file.clear();
    file.seekg(0);

    ofstream file_output("output.bin");
    int count = 0;
    char y = 0;

    //вывод
    while(!file.eof())
    {
        char c;
        c = file.get();
        vector<bool> x = table[c];
        for(int n = 0; n < x.size(); n++)
        {
            //cout << x[n];
            y = y | x[n] << (7 - count);
            count++;
            /*
             * Нужно побитово записать в файл output полчученный код
             * Как только мы достигли 8 битив, записываем его в файл
             * Таким образом, получим блоки из 8 элементов
             */
            if (count == 8)
            {
                count = 0;
                file_output << y; //записываю блок
                y = 0;
            }
        }
    }

    file_output.close(); file.close();

    /*
     * Теперь, нужно написать декодер
     * Нужно, чтобы программа открыла бинарный файл, прочитала его
     * и, проходясь по дереву Хафмана, преобразовала биты в строки
     */
    ifstream file_decode("output.bin");
    ofstream out;
    out.open("out.txt");
    Uzel *p = root;
    int c = 0;
    char tmp;
    file_decode >> y;
    while(!file_decode.eof())
    {
        bool b = y & (1 << (7 - c)); //проверяю первый бит - 0 или 1
        if (b) //если он 1
        {
            p = p->right; //иду вправо
        }
        else // если 0
        {
            p = p->left; // иду влево
        }
        if (p->left == NULL && p->right == NULL) //есди наткнулись на букву
        {
            out << p->symbol; //то вывожу этот символ
            p = root; //снова идём в корень
        }
        c++;
        if (c == 8) //если дошли до 8бита
        {
            c=0; //обнуляем
            file_decode >> y; //считываем новый символ
        }
    }

    file_decode.close(); out.close();
    return 0;
}
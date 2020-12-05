#include <iostream>
#include <stdio.h>
#include <omp.h>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

using namespace std;

int num = 0; //number of coplanar sets
int vectors_quantity = 0; //how many vectors are given, initializes with the Reader
int file_count = 0;
//const int threadNum = 5;
const int threadNum = 10; //number of threads
//const int threadNum = 20;
//const int threadNum = 100;
//const int threadNum = 1000;

//defines if three current vectors are coplanar
bool AreCoplanar(vector<int> v1, vector<int> v2, vector<int> v3) {
    double res = 0;
    omp_set_num_threads(3);
#pragma omp parallel reduction(+: res)
    res += v2[2] * v1[1] * v3[0] + v1[0] * v2[1] * v3[2];
#pragma omp parallel reduction(+: res)
    res += -v1[2] * v2[1] * v3[0] - v2[0] * v1[1] * v3[2];
#pragma omp parallel reduction(+ : res)
    res += v1[2] * v2[0] * v3[1] - v1[0] * v2[2] * v3[1];
    return res == 0;
}

//transforms string vector coords into integer vector coords
vector<int> TransformToVect(string line) {
    vector<int> res;
    res.reserve(3);
    omp_set_num_threads(threadNum);
#pragma omp parallel for private(i)
    for (int i = 0; i < 2; i++) {
        res.push_back(stoi(line.substr(0, line.find(',') + 1)));
        line = line.erase(0, line.find(',') + 1);
    }
    res.push_back(stoi(line));
    return res;
}

//transforms string vector array into vector array
vector<vector<int>> GetVectorArray(string *data, int arr_len) {
    vector<vector<int>> vectors;
    vectors.resize(arr_len);
    omp_set_num_threads(arr_len);
#pragma omp parallel for private(i)
    for (int i = 0; i < arr_len; i++) {
        vectors[i] = TransformToVect(data[i]);
    }
    return vectors;
}

//reads vectors from file
class Reader {
private:
    string pathToInput;
    string *output_arr;
public:
    int quan;

    Reader(string path) {
        pathToInput = path;
    }

    string *GetVectors() {
        ifstream fs(pathToInput);
        if (!(fs.is_open())) {
            throw "Test data file failed to open.";
        }
        string num;
        getline(fs, num);
        quan = stoi(num);
        output_arr = new string[quan];
        omp_set_num_threads(quan);
#pragma omp parallel for private(i)
        for (int i = 0; getline(fs, num); i++) {
            output_arr[i] = num;
        }
        fs.close();
        return output_arr;
    }

    ~Reader(){
        delete[] output_arr;
    }
};

//counts coplanar vector number
void CoplanNumber(vector<vector<int>> arr, int arrSize) {
    omp_set_num_threads((arr.size()-2)/2);
#pragma omp parallel for private(p)
    for (int p = 0; p < threadNum; p++) {
#pragma omp parallel for private(i)
        for (int i = p; i < arrSize - 2; i += threadNum) {
#pragma omp parallel for private(j)
            for (int j = i + 1; j < arrSize - 1; j++)
#pragma omp parallel for private(k)
                    for (int k = j + 1; k < arrSize; k++)
                        if (AreCoplanar(arr[i], arr[j], arr[k])) num++;
        }
    }
}

void OutputResults(long time){
    string name;
    cout << "Enter output file name: ";
    cin >> name;
    string out_path = "output/" + name;
    string result = "Total coplanar vectors: " + to_string(num)
            + "\nExecuted for " + to_string(time) + " ms";
    cout << result;
    ofstream fs(out_path);
    fs << result;
    fs.close();
    cout << "\nFile saved succesfully" << endl;
}

int main() {
    //all the files with vectors should be put into the "input" folder
    //you should write only the name of the file, not its path
    cout << "Enter filename:";
    string path;
    cin >> path;
    //timer start point
    clock_t start_t = clock();
    path = "input/" + path;
    Reader reader = Reader(path);
    string *lines = reader.GetVectors();
    vector<vector<int>> vectorArr = GetVectorArray(lines, reader.quan);
    CoplanNumber(vectorArr, reader.quan);
    clock_t end_t = clock();
    OutputResults(end_t-start_t);
    return 0;
}

#include <iostream>
#include <vector>

int* adjacent_swap(const int* arr, size_t n){
    int *ar = new int[n];
    for(size_t i = 0; i < n; i+=2){
        ar[i] = arr[i+1];
        ar[i+1] = arr[i];
    }
    return ar;
}

std::vector<int> adjacent_swap(const std::vector<int>& arr){
    std::vector<int> ar = arr;
    size_t len = arr.size();
    for(size_t i = 0; i < len; i+=2){
        ar[i] = arr[i+1];
        ar[i+1] = arr[i];
    }
    return ar;
}
/*
int main(){
    std::vector a = {1, 2, 3, 4, 5, 6};
    std::vector k = adjacent_swap(a);
    for (int i = 0; i < 6;i++){
        std::cout << k[i] << std::endl;
    }
    return 0;
}
*/
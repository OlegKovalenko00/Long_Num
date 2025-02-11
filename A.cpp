#include <algorithm>
#include <vector>
#include <date.h>
#include <string>

void sort_dates(std::vector<Date> &dates, std::string_view order) {
    std::sort(dates.begin(), dates.end(),
    
    [order](const Date &d1, const Date &d2) -> bool { 
        for(char c : order) {
            switch(c) {
                case 'y':
                    if (d1.year != d2.year) return d1.year < d2.year;
                    break;
                case 'm':
                    if (d1.month != d2.month) return d1.month < d2.month;
                    break;
                case 'd':
                    if (d1.day != d2.day) return d1.day < d2.day;
                    break;
            }
        }
        return false;
    }
        );
}
/*
int main(){
    return 0;
}
*/
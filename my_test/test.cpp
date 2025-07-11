#include <absl/strings/str_cat.h>
#include <iostream>
#include <curl/curl.h>
int main() {
    std::string info = "User: ";
    int id = 123;

    // 追加内容
    absl::StrAppend(&info, "ID-", id, ", Active: ", true);
    
    std::cout << info << std::endl;
    // // 输出: User: ID-123, Active: true
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int  port_client = 5500;
#define BUF_SIZE 1024
char IP_client[1000];

// Hàm kiểm tra định dạng số port 
int validatePort(const char* port) {
    int length = strlen(port);

    for (int i = 0; i < length; i++) {
        if (!isdigit((unsigned char)port[i])) {
            return 0;  // Không phải ký tự số
        }
    }
    return 1;  // Hợp lệ
}

// kiem tra xem dia chi ip co hop le khong 
int check_ip(char *ip) {
    int num, dots = 0;
    char *ptr;

    // Kiểm tra nếu chuỗi IP là NULL
    if (ip == NULL) {
        return 0;
    }

    // Tách chuỗi IP bằng dấu chấm
    ptr = strtok(ip, ".");

    while (ptr) {
        // Kiểm tra xem phần này có phải là một số hợp lệ không
        if (*ptr == '\0' || !isdigit(*ptr)) {
            return 0;
        }

        // Kiểm tra nếu tất cả ký tự đều là số
        for (int i = 0; ptr[i] != '\0'; i++) {
            if (!isdigit(ptr[i])) {
                return 0; // Không phải ký tự số
            }
        }

        // Chuyển chuỗi sang số nguyên
        num = atoi(ptr);

        // Kiểm tra xem số có nằm trong khoảng từ 0 đến 255 không
        if (num < 0 || num > 255) {
            return 0; // Không hợp lệ
        }

        // Chuyển đến phần tiếp theo
        ptr = strtok(NULL, ".");
        dots++; // Đếm số dấu chấm
    }

    // Địa chỉ IP hợp lệ nếu có đúng 3 dấu chấm
    if (dots != 4) {
        return 0; // Không hợp lệ
    }

    return 1; // Hợp lệ
}

int kiemTraInput(char* str[]) {
    // Bước 1: Kiểm tra xem có "./client" ở đầu chuỗi không
    if (strcmp(str[0], "./client") != 0) {
        printf("Sai cu phap: Chuoi phai bat dau bang './client'.\n");
        return 0;
    }

    // Bước 2: Kiểm tra công Post có hợp lệ không 
    if(validatePort(str[2]) != 1){
        printf("So cong Port khong hop le.\n");
        return 0;
    }

    // Bước 3: Kiểm tra địa chỉ IP
    char IP[1000];
    strcpy(IP, str[1]);
    if(check_ip(IP) != 1){
        printf("Dia chi ip khong hop le.\n");
        return 0;
    }

    // Nếu tất cả đều thỏa mãn
    return 1;
}

void input_Port_Ip(){
    char input[1000];

    while(1){
        // Nhap cu phap   
        printf("Nhap theo cau truc sau: ./client IPAddress PortNumber\n");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  // xóa ký tự newline 

        // tách chuỗi input thành các thành phần 
        char* str[10];
        char* token = strtok(input, " ");
        int index = 0;

        // lay tung thanh phan cua input
        while (token != NULL && index < 10) {
            str[index++] = token;
            token = strtok(NULL, " ");
        }

        // Thay đổi kiểm tra index
        if(index != 3 || kiemTraInput(str) == 0){
            printf("Nhap sai cu phap. Phai nhap theo dinh dang: ./client IPAddress PortNumber\n");
            printf("\n");
            continue;
        } else {
            port_client = atoi(str[2]); // Gán số cổng
            strcpy(IP_client, str[1]); // Gán địa chỉ IP
            break; // Thoát khỏi vòng lặp
        }
    }
}    


int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUF_SIZE];
    
    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Tạo socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Nhap dia chi ip va so cong 
    input_Port_Ip();


    // Thiết lập địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP_client);
    serverAddr.sin_port = htons(port_client);

    while (1) {
        // Nhập thông điệp từ người dùng
        printf("-----------------------------------\n");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự newline

        // Gửi thông điệp đến server
        sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        // Kiểm tra xem buffer có chứa toàn bộ khoảng trắng không
        int n1 = strlen(buffer);
        int key = 0; // Giả định rằng tất cả đều là khoảng trắng
        for (int i = 0; i < n1; i++) {
            if (buffer[i] != ' ') {
                key = 1; // Có ít nhất một ký tự không phải là khoảng trắng
                break;
            }
        }

        if(key == 0){
            // Đóng socket và giải phóng Winsock
            closesocket(clientSocket);
            WSACleanup();
            return 0;
        }

        // Nhận phản hồi từ server
        int n = recvfrom(clientSocket, buffer, BUF_SIZE, 0, NULL, NULL);
        if (n == SOCKET_ERROR) {
            printf("recvfrom failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        buffer[n] = '\0'; // Kết thúc chuỗi
        printf("%s\n", buffer);
        printf("-------------------------------\n");
    }

    // Đóng socket và giải phóng Winsock
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <stdlib.h>  
#include <ctype.h> 
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define BUF_SIZE 1028
int port_server;

// Định nghĩa struct Time
typedef struct Time {
    char userName[1000];
    char ngay[11];  // Chuỗi lưu ngày: dd/mm/yyyy (10 ký tự + '\0')
    char gio[9];    // Chuỗi lưu giờ: hh:mm:ss (8 ký tự + '\0')

    struct Time* next;
} Time;

// Hàm tạo một Time mới
Time* createTime(const char*userName, const char* ngay, const char* gio){

    Time* newTime = (Time*)malloc(sizeof(Time));

    if (newTime == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    // Sao chép dữ liệu vào node
    strncpy(newTime->ngay, ngay, sizeof(newTime->ngay) - 1);
    newTime->ngay[sizeof(newTime->ngay) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newTime->gio, gio, sizeof(newTime->gio) - 1);
    newTime->gio[sizeof(newTime->gio) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newTime->userName, userName, sizeof(newTime->userName) - 1);
    newTime->userName[sizeof(newTime->userName) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    
    return newTime;
}

// Hàm chèn một Time mới vào cuối danh sách 
void insertTimeAtTail(Time** dau, const char* userName, const char* ngay, const char* gio){
    Time* newTime = createTime(userName, ngay, gio);

    if(*dau == NULL){
        *dau = newTime;
        return;
    }

    // Duyệt tới node cuối cùng
    Time* current = *dau;
    while (current->next != NULL) {
        current = current->next;
    }

    // Gán node mới vào cuối danh sách
    current->next = newTime;
}

// Hàm giải phóng bộ nhớ của danh sách liên kết Time
void freeListTime(Time* dau) {
    Time* temp;
    while (dau != NULL) {
        temp = dau;
        dau = dau->next;
        free(temp);
    }
}

// dinh nghia Struct luu so lan dang nhap con lai
typedef struct Login {

    char userName[1000];
    int x;

    struct Login* next;
} Login;

// tao 1 loGin moi
Login* createLogin(const char* userName){
    Login* newLogin = (Login*)malloc(sizeof(Login));

    strcpy(newLogin->userName, userName);
    newLogin->x = 3;
    newLogin->next = NULL;

    return newLogin;
}

// Hàm chèn một login mới vào cuối danh sách 
void insertLoginAtTail(Login** dangnhap, const char* userName) {
    Login* newLogin = createLogin(userName);

    if (*dangnhap == NULL) {
        *dangnhap = newLogin;
        return;
    }

    // Duyệt tới node cuối cùng
    Login* current = *dangnhap;
    while (current->next != NULL) {
        current = current->next;
    }

    // Gán node mới vào cuối danh sách
    current->next = newLogin;
}

// xoa 1 node dua vao userName
void deleteLoginByUserName(Login** dangnhap, const char* userName) {
    // Kiểm tra nếu danh sách rỗng
    if (*dangnhap == NULL) {
        printf("Danh sách rỗng, không thể xóa.\n");
        return;
    }

    Login* current = *dangnhap;
    Login* previous = NULL;

    // Kiểm tra nếu node đầu tiên là node cần xóa
    if (strcmp(current->userName, userName) == 0) {
        *dangnhap = current->next;  // Di chuyển con trỏ đầu danh sách
        free(current);               // Giải phóng bộ nhớ của node
        printf("Đã xóa thành công userName: %s\n", userName);
        return;
    }

    // Duyệt qua danh sách để tìm node cần xóa
    while (current != NULL && strcmp(current->userName, userName) != 0) {
        previous = current;          // Giữ lại node trước node cần xóa
        current = current->next;     // Di chuyển đến node tiếp theo
    }

    // Nếu không tìm thấy node có userName cần xóa
    if (current == NULL) {
        printf("Không tìm thấy userName: %s trong danh sách.\n", userName);
        return;
    }

    // Bỏ qua node cần xóa
    previous->next = current->next;
    free(current);                   // Giải phóng bộ nhớ của node
    printf("Đã xóa thành công userName: %s\n", userName);
}

// Hàm giải phóng bộ nhớ của danh sách liên kết login
void freeListLogin(Login* dangnhap) {
    Login* temp;
    while (dangnhap != NULL) {
        temp = dangnhap;
        dangnhap = dangnhap->next;
        free(temp);
    }
}

// doc du lieu tu file va tao 1 danh sach Login
void docFileLogin(Login** dangnhap, const char* fileName){
        
    char userName[1000];
    char password[1000];
    char email[1000];
    char phone[1000];
    char homePage[1000];
    int status; 

    // Mở file nguoidung.txt để tạo danh sách liên kết 
    FILE* file = fopen(fileName, "r");  
    if(file == NULL){
        printf("Không thể mở file %s\n", fileName);
        return;
    }

    while(fscanf(file, "%s %s %s %s %d %s", userName, password, email, phone, &status, homePage) == 6){
        if(status == 0) continue;
        insertLoginAtTail(dangnhap,userName);
    }

    // Đóng file 
    fclose(file);
}

// Định nghĩa cấu trúc của một node
typedef struct Node {

    char userName[1000];
    char password[1000];
    char email[1000];
    char phone[1000];
    int status;
    char homePage[1000];

    struct Node* next;
} Node;

// Hàm tạo một node mới
Node* createNode(const char* userName, const char* password, const char* email, const char* phone, int status, char* homepage) {
    
    Node* newNode = (Node*)malloc(sizeof(Node));
    
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    // Sao chép dữ liệu vào node
    strncpy(newNode->userName, userName, sizeof(newNode->userName) - 1);
    newNode->userName[sizeof(newNode->userName) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newNode->password, password, sizeof(newNode->password) - 1);
    newNode->password[sizeof(newNode->password) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newNode->email, email, sizeof(newNode->email) - 1);
    newNode->email[sizeof(newNode->email) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newNode->phone, phone, sizeof(newNode->phone) - 1);
    newNode->phone[sizeof(newNode->phone) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    strncpy(newNode->homePage, homepage, sizeof(newNode->homePage) - 1);
    newNode->homePage[sizeof(newNode->homePage) - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    newNode->status = status;
    newNode->next = NULL;
    return newNode;
}

// Hàm chèn một node mới vào cuối danh sách
void insertAtTail(Node** head, const char* userName, const char* password, const char* email, const char* phone, int status, char* homepage) {
    Node* newNode = createNode(userName, password, email, phone, status, homepage);

    // Nếu danh sách rỗng, node mới sẽ là node đầu tiên
    if (*head == NULL) {
        *head = newNode;
        return;
    }

    // Duyệt tới node cuối cùng
    Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    // Gán node mới vào cuối danh sách
    current->next = newNode;
}

// Hàm giải phóng bộ nhớ của danh sách liên kết Node
void freeList(Node* head) {
    Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// Hàm kiểm tra userName đã tồn tại hay chưa
int checkUserNameExist(Node* head, const char* userName) {
    Node* current = head;
    while (current != NULL) {

        // So sánh userName trong danh sách với userName nhập vào
        if (strcmp(current->userName, userName) == 0) {
            return 1; // Tìm thấy userName trùng khớp
        }
        current = current->next;
    }
    return 0; // Không tìm thấy userName
}

// Hàm đọc file nguoidung.txt
void docFile(Node** head, const char* fileName){

    char userName[1000];
    char password[1000];
    char email[1000];
    char phone[1000];
    char homePage[1000];
    int status; 

    // Mở file nguoidung.txt để tạo danh sách liên kết 
    FILE* file = fopen(fileName, "r");  
    if(file == NULL){
        printf("Không thể mở file %s\n", fileName);
        return;
    }

    while(fscanf(file, "%s %s %s %s %d %s", userName, password, email, phone, &status, homePage) == 6){
        insertAtTail(head,userName, password, email, phone, status, homePage);
    }

    // Đóng file 
    fclose(file);

}

// Kiểm tra đăng nhập 
int kiemTraSign(Node* head, const char* userName, const char* password){

    Node* current = head;
    while (current != NULL) {

        // So sánh userName, password trong danh sách với userName, password nhập vào
        if (strcmp(current->userName, userName) == 0 && strcmp(current->password, password) == 0) {
            return 1; // Tìm thấy tai khoan 
        }
        current = current->next;
    }
    return 0; // Không tìm thấy tai khoan
}

// Kiểm tra tài khoản có bị khóa
int trangThaiKhoa(Node* head, const char* userName){

    Node* current = head;
    while (current != NULL) {

        // Kiểm tra sự tồn tại của userName, và kiểm tra trạng thái của tài khoản
        if (strcmp(current->userName, userName) == 0 && current->status == 0) {
            return 1; // Tài khoản đang bị khóa
        }
        current = current->next;
    }
    return 0; // Tài khoản không bị khóa

}

// Khóa tài khoản 
void khoaTaiKhoan(Node* head, const char* userName){

    Node* p = head;

    while(p != NULL){

        if(strcmp(p->userName, userName) == 0){
            p->status = 0;
            break;
        }
        p = p->next;
    }
}

// Update trạng thái hoạt động 
void updateHoatDong(Node* head, const char* userName){

    Node* p = head;

    while(p != NULL){

        if(strcmp(p->userName, userName) == 0){
            p->status = 1;
            break;
        }
        p = p->next;
    }
}

// Update trạng thái không hoạt động 
void updateKhongHoatDong(Node* head, const char* userName){

    Node* p = head;

    while(p != NULL){

        if(strcmp(p->userName, userName) == 0){
            p->status = 2;
            break;
        }
        p = p->next;
    }

    // cập nhật trạng thái không hoạt động của tài khoản trong file nguoidung.txt
    FILE* file = fopen("nguoidung.txt", "w");

    if (file == NULL) {
        printf("Không thể mở file %s\n", "nguoidung.txt");
        return;
    }

    Node* temp = head;
    while (temp != NULL) {
        fprintf(file, "%s %s %s %s %d %s\n", temp->userName, temp->password, temp->email, temp->phone, temp->status, temp->homePage);
        temp = temp->next;
    }

    fclose(file);
}

// Đổi mật khẩu
void changePassword(Node* head, char* userName, char* password){

    char matKhauMoi[1000];
    char matKhauCu[1000];

    while(1){

        printf("Nhap mat khau cu: ");
        fgets(matKhauCu, sizeof(matKhauCu), stdin);
        matKhauCu[strcspn(matKhauCu, "\n")] = 0;  // Xóa ký tự newline

        printf("Nhap mat khau moi: ");
        fgets(matKhauMoi, sizeof(matKhauMoi), stdin);
        matKhauMoi[strcspn(matKhauMoi, "\n")] = 0;  // Xóa ký tự newline

        if(strcmp(matKhauCu, password) == 0){
            break;
        }else{
            printf("Nhap sai mat khau.\n");
        }
    }

    Node* p = head;
    while(p != NULL){

        if(strcmp(p->userName, userName) == 0){
            strcpy(p->password, matKhauMoi);
            break;
        }
        p = p->next;
    }

    FILE* file = fopen("nguoidung.txt", "w");

    if (file == NULL) {
        printf("Không thể mở file %s\n", "nguoidung.txt");
    return;
    }

    Node* temp = head;
    while (temp != NULL) {
        fprintf(file, "%s %s %s %s %d %s\n", temp->userName, temp->password, temp->email, temp->phone, temp->status, temp->homePage);
        temp = temp->next;
    }

    fclose(file);
}

// kiem tra input tu client 
int check_input_client(char* input){
        
        // tách chuỗi input thành các thành phần 
        char* str[10];
        char* token = strtok(input, " ");
        int index = 0;

        // lay tung thanh phan cua input
        while (token != NULL)
        {
            str[index++] = token;
            token = strtok(NULL, " ");
        }

        if(index > 2){
            return 3;
        }else if(index == 2){
            return 2;
        }else{
            return 1;
        }
}

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

int kiemTraInput(char* str[]) {
    // Bước 1: Kiểm tra xem có "./server" ở đầu chuỗi không
    if (strcmp(str[0], "./server") != 0) {
        printf("Sai cu phap: Chuoi phai bat dau bang './server'.\n");
        return 0;
    }

    // Bước 2: Kiểm tra công Post có hợp lệ không 
    if(validatePort(str[1])!=1){
        printf("So cong Port khong hop le.\n");
        return 0;
    }

    // Nếu tất cả đều thỏa mãn
    return 1;
}

void inputPort(){
    char input[1000];

    while(1){

        // Nhap cu phap   
        printf("Nhap theo cau truc sau: ./server PortNumber\n");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  // xóa ký tự newline 

        // tách chuỗi input thành các thành phần 
        char* str[10];
        char* token = strtok(input, " ");
        int index = 0;

        // lay tung thanh phan cua input
        while (token != NULL)
        {
            str[index++] = token;
            token = strtok(NULL, " ");
        }

        if(index != 2 || kiemTraInput(str) == 0){
            printf("Nhap sai cu phap. Phai nhap theo dinh dang: ./server PortNumber\n");
            printf("\n");
            continue;
        }else{

            port_server = atoi(str[1]);
            break;
        }
    }
}

// Hàm kiểm tra ký tự đặc biệt
int containsSpecialCharacters(const char *str) {
    while (*str != '\0') {
        if (!isalnum((unsigned char)*str)) {  // Nếu ký tự không phải là chữ hoặc số
            return 1;  // Chuỗi có chứa ký tự đặc biệt
        }
        str++;
    }
    return 0;  // Không có ký tự đặc biệt
}

void splitString(const char *input, char *letters, char *digits) {
    // Khởi tạo các chuỗi chữ cái và số rỗng
    letters[0] = '\0';
    digits[0] = '\0';

    // Duyệt từng ký tự trong chuỗi đầu vào
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalpha((unsigned char)input[i])) {
            // Nếu là chữ cái, thêm vào chuỗi letters
            strncat(letters, &input[i], 1);
        } else if (isdigit((unsigned char)input[i])) {
            // Nếu là số, thêm vào chuỗi digits
            strncat(digits, &input[i], 1);
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    char buffer[BUF_SIZE];

    int j=0;
    Node* head = NULL;
    Time* dau = NULL;
    Login* dangnhap = NULL;

    // Đọc file account và xây dựng danh sách 
    docFile(&head, "nguoidung.txt");
    // Doc file nguoidung.txt xay dung danh sach login 
    docFileLogin(&dangnhap, "nguoidung.txt");
    
    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Tạo socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Nhập số cổng PORT cho server 
    inputPort();

    // Thiết lập địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_server);

    // Gắn địa chỉ cho socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d...\n", port_server);

    char UserName[1000];
    char passWord[1000];

    while (1) {
        // Nhận thông điệp từ client
        int n = recvfrom(serverSocket, buffer, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, &addrLen);
        printf("%s\n", buffer);
        if (n == SOCKET_ERROR) {
            printf("recvfrom failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        buffer[n] = '\0'; // Kết thúc chuỗi

        // lam viec voi yeu cau buffer cua client 
        char buffer2[1000];
        strcpy(buffer2, buffer);

        // Kiểm tra xem buffer có chứa toàn bộ khoảng trắng không
        int key = 0; // Giả định rằng tất cả đều là khoảng trắng
        for (int i = 0; i < n; i++) {
            if (buffer[i] != ' ') {
                key = 1; // Có ít nhất một ký tự không phải là khoảng trắng
                break;
            }
        }

        if(key != 0){
            key = check_input_client(buffer2);  // key=0 => thoat, key=1 => doimatkhau, logout, homepage, key = 2 => login
        }                                       // key = 3 => nhap thua so truong can thiet

        printf("%d\n", key);
        switch (key)
        {
        case 0:
            
            // Đóng socket và giải phóng Winsock
            closesocket(serverSocket);
            WSACleanup();
            return 0;
        case 1:

            if(j == 0){
                // Gửi phản hồi "chua dang nhap" cho client
                const char *response = "Insert password";
                sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                break;
            }else{

                char* logout = "bye";
                char* homepage = "homepage";

                if(strcmp(logout, buffer) == 0){        // dang xuat;
                    char str[1000] = "Goodbye ";
                    strcat(str, UserName);

                    printf("%s\n", str);

                    sendto(serverSocket, str, strlen(str), 0, (struct sockaddr*)&clientAddr, addrLen);
                    j = 0;
                    break;
                }else if(strcmp(homepage, buffer) == 0){        // hien thi homepage cua tai khoan 

                    Node* p = head;
                    while (p != NULL)
                    {
                        if(strcmp(p->userName, UserName) == 0){
                            break;
                        }
                        p = p->next;
                    }

                    char xtr[1000];
                    strcpy(xtr, p->homePage);
                    sendto(serverSocket, xtr, strlen(xtr), 0, (struct sockaddr*)&clientAddr, addrLen);  
                    break;
                }else{              // doi mat khau cho tai khoan 
                    int h = containsSpecialCharacters(buffer);

                    // chuoi co ky tu dac biet => Error
                    if(h == 1){
                        const char *response = "Error";
                        sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                        break;
                    }else{

                        char pass_str[1000];
                        char pass_int[1000];
                        char buffer3[1000];

                        strcpy(buffer3, buffer);

                        splitString(buffer3, pass_str, pass_int);
                        strcat(pass_int, "\n");
                        strcat(pass_int, pass_str);
                        sendto(serverSocket, pass_int, strlen(pass_int), 0, (struct sockaddr*)&clientAddr, addrLen);

                        Node* p = head;
                        while(p != NULL){
                            if(strcmp(p->userName, UserName) == 0){
                                strcpy(p->password, buffer);
                                break;
                            }
                            p = p->next;
                        }

                        FILE* file = fopen("nguoidung.txt", "w");
                        if (file == NULL) {
                            printf("Không thể mở file %s\n", "nguoidung.txt");
                        break;
                        }

                        Node* temp = head;
                        while (temp != NULL) {
                                fprintf(file, "%s %s %s %s %d %s\n", temp->userName, temp->password, temp->email, temp->phone, temp->status, temp->homePage);
                            temp = temp->next;
                        }

                        fclose(file);
                        break;

                    }
                }
            }
            break;
        case 2:             // login 

            if(j == 1){
                char* vtr= "Hay logout truoc khi dang nhap lan nua.";
                sendto(serverSocket, vtr, strlen(vtr), 0, (struct sockaddr*)&clientAddr, addrLen);
                break;
            }
            char buffer4[1000];
            strcpy(buffer4, buffer);
            char* str[10];

            char* token = strtok(buffer4, " ");
            int index = 0;

            // Tách từng phần của input
            while (token != NULL && index < 10) {
                str[index] = token;
                index++;
                token = strtok(NULL, " ");
            }

            strcpy(UserName, str[0]);
            strcpy(passWord, str[1]);
            char UserName2[1000];
            char passWord2[1000];

            strcpy(UserName2, str[0]);
            strcpy(passWord2, str[1]);
            printf("%s, %s\n", UserName, passWord);
            // int o = Sign(serverSocket, &dau, &head, UserName, sizeof(UserName), passWord, sizeof(passWord), (const struct sockaddr *)&clientAddr, addrLen, &j);
            
            char ngay[1000];
            char gio[1000];

            // Nhập userName và kiểm tra 
            if(checkUserNameExist(head, UserName2)){   

                if(trangThaiKhoa(head, UserName2)){

                    // Gửi phản hồi lại cho client tai khoan da bi khoa
                    const char *response = "account not ready.";
                    sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                    break;
                }
                
            }else{       
                // Gửi phản hồi lại cho client ten userName khong dung 
                const char *response = "User Name khong dung.";
                sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                break;
            }         
            // Nhập password và kiểm tra  

            if(kiemTraSign(head, UserName2, passWord2)){ // Đăng nhập thành công 
                
                // cap nhat so lan login con lai cua userName la 3
                Login* l = dangnhap;
                while( l!= NULL){

                    if(strcmp(l->userName, UserName2) == 0){
                        l->x = 3;
                        break;
                    }
                    l = l->next;
                }
                // Lấy thời gian hiện tại
                time_t now = time(NULL);
                struct tm* local = localtime(&now);

                // Lưu thông tin ngày vào chuỗi 'ngay'
                sprintf(ngay, "%02d/%02d/%04d", local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
                // Lưu thông tin giờ vào chuỗi 'gio'
                sprintf(gio, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);

                insertTimeAtTail(&dau, UserName2, ngay, gio);
                    
                FILE* hile = fopen("history.txt", "a");

                if (hile == NULL) {
                    printf("Không thể mở file %s\n", "history.txt");
                    break;
                }

                Time* t = dau;
                while (t != NULL) {
                    if(strcmp(t->userName, UserName2) == 0){
                        fprintf(hile, "%s|%s|%s\n", t->userName, t->ngay, t->gio);
                    }
                    t = t->next;
                }

                fclose(hile);
                updateHoatDong(head, UserName2);

                // cập nhật trạng thái hoạt động của vào file nguoidung.txt
                FILE* file = fopen("nguoidung.txt", "w");

                if (file == NULL) {
                    printf("Không thể mở file %s\n", "nguoidung.txt");
                    break; 
                }

                Node* temp = head;
                while (temp != NULL) {
                    fprintf(file, "%s %s %s %s %d %s\n", temp->userName, temp->password, temp->email, temp->phone, temp->status, temp->homePage);
                    temp = temp->next;
                }

                fclose(file);

                // Gửi phản hồi lại cho client ok
                const char *response = "OK";
                sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                j = 1;
                break;
            }else {
                
                Login* k = dangnhap;
                while(k != NULL){
                    
                    if(strcmp(k->userName, UserName2) == 0){
                        break;
                    }
                    k = k->next;
                } 

                if(k->x == 0){
                    
                    khoaTaiKhoan(head, UserName2);
                    FILE* file = fopen("nguoidung.txt", "w");

                    if (file == NULL) {
                        printf("Không thể mở file %s\n", "nguoidung.txt");
                    break;
                    }

                    Node* temp = head;
                    while (temp != NULL) {
                        fprintf(file, "%s %s %s %s %d %s\n", temp->userName, temp->password, temp->email, temp->phone, temp->status, temp->homePage);
                        temp = temp->next;
                    }

                    fclose(file);

                    const char *response = "block account.";
                    sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                }else{
                    k->x = k->x - 1;
                    const char *response = "not OK.";
                    sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
                }
            }

            break;
        case 3:
            
            const char *response = "Nhap thua so truong.";
            sendto(serverSocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, addrLen);
            break;
        default:
            break;
        }
    }

    // Đóng socket và giải phóng Winsock
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

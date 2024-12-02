#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

#include "authenticate.h"
#include "color.h"
#include "command.h"
#include "common.h"
#include "connect.h"
#include "crypto.h"
#include "log.h"
#include "utils.h"
#include "validate.h"

const char *process;
char root_dir[SIZE];

std::string SYMMETRIC_KEY = "";

/**
 * Đọc lệnh từ người dùng và lưu trữ trong cấu trúc `Message`
 * @param user_input chuỗi lệnh do người dùng nhập
 * @param size kích thước tối đa của chuỗi lệnh
 * @param msg con trỏ đến cấu trúc `Message` để lưu trữ lệnh
 * @return 0 nếu thành công, -1 nếu có lỗi
 */
int cli_read_command(char *user_input, int size, Message *msg);

/**
 * Xử lý gửi khóa đối xứng đến server
 * 1. Tạo một khóa đối xứng (symmetric key)
 * 2. Nhận khóa công khai từ server
 * 3. Mã hóa khóa đối xứng bằng khóa công khai
 * 4. Gửi khóa đối xứng đã mã hóa đến server
 * @param sockfd socket điều khiển để giao tiếp với server
 */
void handle_symmetric_key_pair(int sockfd);

/**
 * Hàm chính của chương trình
 * 1. Kết nối đến server bằng socket
 * 2. Hiển thị menu chính (Login, Register, Exit)
 * 3. Xử lý đăng nhập, đăng ký tài khoản
 * 4. Sau khi đăng nhập thành công, thực hiện các lệnh liên quan đến quản lý file:
 *    - Gửi lệnh đến server
 *    - Thực hiện tải lên/tải xuống, tìm kiếm, chia sẻ file
 *    - Xử lý thư mục (cd, pwd, mkdir,...)
 *    - In ra thông báo lỗi hoặc kết quả thực thi
 * 5. Đóng kết nối socket khi thoát chương trình
 */

// input địa chỉ IP và số cổng của server
int main(int argc, char const *argv[]) {
    process = argv[0];  // Lưu trữ tên của chương trình vào biến process
    int sockfd;
    int data_sock;
    char user_input[SIZE];
    char *cur_user = (char *) malloc(sizeof(char) * SIZE);  // Cấp phát bộ nhớ cho tên người dùng
    char *log_msg = (char *) malloc(sizeof(char) * SIZE * 2);  // Cấp phát bộ nhớ cho thông báo log

    if (argc != 3) {  // Kiểm tra nếu số tham số dòng lệnh không đúng
        printf("Usage: %s <ip_adress> <port>\n", argv[0]);
        exit(0);
    }

    if (validate_ip(argv[1]) == INVALID_IP) {  // Kiểm tra địa chỉ IP hợp lệ
        printf("Error: Dia chi IP khong hop le.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // Tạo socket TCP

    if (sockfd == -1) {  // Kiểm tra nếu socket tạo không thành công
        perror("Error: Tao socket that bai.");
        exit(1);
    }

    struct sockaddr_in sock_addr;

    // cấu hình địa chỉ cho server
    sock_addr.sin_family = AF_INET;  
    sock_addr.sin_port = htons(atoi(argv[2]));  
    sock_addr.sin_addr.s_addr = inet_addr(argv[1]); 

    // kết nối server
    int connect_status = connect(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr));  

    if (connect_status == -1) {  // Kiểm tra nếu kết nối thất bại
        printf("Error: Ket noi server that bai!\n");
        exit(1);
    }

    const char *menu_items[] = {"Login", "Register", "Exit"}; 

    // hiển thị menu "đăng nhập, đăng ký, thoát"
    int choice = process_menu(menu_items, 3);  
    
    int fl = 1;
    switch (choice) {
        case 0:  // đăng nhập 
            do {
                print_centered("Login to cppdrive");  // Hiển thị thông báo đăng nhập
                fl = handle_login(sockfd, cur_user);  // Xử lý đăng nhập cur_user lưu userName đăng nhập thành công 
            } while (!fl);  // Tiếp tục cho đến khi đăng nhập thành công
            break;
        case 1:  // Đăng ký 
            do {
                print_centered("Register new account");  // Hiển thị thông báo đăng ký
                fl = register_acc(sockfd);  // Xử lý đăng ký tài khoản mới
            } while (!fl);  // Tiếp tục cho đến khi đăng ký thành công
            do {
                print_centered("Login to cppdrive");  // Sau khi đăng ký xong, yêu cầu đăng nhập
                fl = handle_login(sockfd, cur_user);
            } while (!fl);
            break;
        case 2:  // Thoát
            print_centered("Good bye!");  
            exit(0);  // Thoát chương trình
            break;
    }
    sprintf(log_msg, "User %s login successfully", cur_user);  // Ghi log khi người dùng đăng nhập thành công
    log_message('i', log_msg);

    handle_symmetric_key_pair(sockfd);  // Thiết lập cặp khóa đối xứng

    // Bắt đầu vòng lặp shell
    char *user_dir = (char *) malloc(SIZE);  // Biến chứa thư mục người dùng
    strcpy(user_dir, "~/");  // Mặc định thư mục là ~/ 
    int error = 0;
    while (1) {
        char *prompt = handle_prompt(cur_user, user_dir);  // Lấy prompt cho shell
        if (error) {
            printf(ANSI_COLOR_RED "%s" ANSI_RESET, prompt);  // Hiển thị prompt với màu đỏ nếu có lỗi
        }
        printf(ANSI_COLOR_GREEN "%s" ANSI_RESET, prompt);  // Hiển thị prompt với màu xanh nếu không có lỗi
        fflush(stdout);
        Message command;
        memset(command.payload, 0, sizeof(command.payload));  // Xóa nội dung của command payload
        int fl = cli_read_command(user_input, sizeof(user_input), &command);  // Đọc lệnh từ người dùng
        command.length = strlen(command.payload);  // Cập nhật chiều dài lệnh
        if (fl == -1) {  // Nếu lệnh không hợp lệ
            printf("Invalid command\n");
            sprintf(log_msg, "User %s enter invalid command", cur_user);  // Ghi log về lệnh không hợp lệ
            log_message('e', log_msg);
            continue;
        }
        sprintf(log_msg, "User %s enter command: %s", cur_user, user_input);  // Ghi log lệnh người dùng nhập
        log_message('i', log_msg);

        // Gửi lệnh đến server
        if (send_message(sockfd, command) < 0) {  // Nếu gửi lệnh không thành công
            sprintf(log_msg, "User %s send command failed", cur_user);
            log_message('e', log_msg);
            close(sockfd);  // Đóng kết nối socket
            exit(1);
        }

        // Mở kết nối dữ liệu
        if ((data_sock = client_start_conn(sockfd)) < 0) {  // Mở socket kết nối dữ liệu
            perror("Error opening socket for data connection");
            exit(1);
        }

        // Xử lý các loại lệnh khác nhau
        if (command.type == MSG_TYPE_QUIT) {  // Nếu lệnh là "quit"
            sprintf(log_msg, "User %s exit", cur_user);  // Ghi log khi người dùng thoát
            log_message('i', log_msg);
            printf("Goodbye.\n");
            break;
        } else if (command.type == MSG_TYPE_CLEAR) {  // Nếu lệnh là "clear"
            system("clear");  // Xóa màn hình
            continue;
        } else if (command.type == MSG_TYPE_LS) {  // Nếu lệnh là "ls"
            handle_list(data_sock);  // Xử lý lệnh liệt kê thư mục
        } else if (command.type == MSG_TYPE_BASIC_COMMAND) {  // Xử lý các lệnh cơ bản
            Message response;
            recv_message(sockfd, &response);  // Nhận phản hồi từ server
            if (response.type == MSG_TYPE_ERROR) {  // Nếu có lỗi
                sprintf(log_msg, "Command output: %s", response.payload);
                log_message('w', log_msg);
                printf(ANSI_COLOR_YELLOW "%s" ANSI_RESET "\n", response.payload);
                continue;
            }
            while (1) {
                recv_message(sockfd, &response);  // Nhận phản hồi tiếp theo
                if (response.type == MSG_TYPE_ERROR) {  // Nếu có lỗi
                    sprintf(log_msg, "Command output: %s", response.payload);
                    log_message('e', log_msg);
                    printf(ANSI_COLOR_RED "%s\n" ANSI_RESET,
                           "Something went wrong!\nCheck log file for more details!");
                    break;
                } else if (response.type == MSG_DATA_CMD) {  // Nếu có dữ liệu trả về
                    printf("%s", response.payload);
                } else {
                    sprintf(log_msg, "Command run sucessfully");
                    log_message('i', log_msg);
                    break;
                }
            }
        } else if (command.type == MSG_TYPE_CD) {  // Nếu lệnh là "cd"
            Message response;
            recv_message(sockfd, &response);
            switch (response.type) {
                case MSG_DATA_CD:
                    sprintf(log_msg, "Command output: %s", response.payload);
                    log_message('i', log_msg);
                    strcpy(user_dir, response.payload);  // Cập nhật thư mục người dùng
                    break;
                case MSG_TYPE_ERROR:
                    printf("%s\n", response.payload);
                    sprintf(log_msg, "Command output: %s", response.payload);
                    log_message('e', log_msg);
                    break;
                default:
                    break;
            }
        } else if (command.type == MSG_TYPE_DOWNLOAD) {  // Nếu lệnh là "download"
            handle_download(data_sock, sockfd, command.payload);  // Xử lý tải xuống
        } else if (command.type == MSG_TYPE_FIND) {  // Nếu lệnh là "find"
            Message response;
            std::string files;
            while (1) {
                recv_message(sockfd, &response);  // Nhận phản hồi từ server
                if (response.type == MSG_TYPE_ERROR)
                    printf("%s", response.payload);
                else if (response.type == MSG_DATA_FIND) {
                    printf("%s", response.payload);
                    std::string str(response.payload);
                    files += str;
                } else {
                    break;
                }
            }
            recv_message(sockfd, &response);
            if (response.type == MSG_TYPE_PIPE) {  // Nếu có phản hồi về pipe
                handle_pipe_download(sockfd, files);  // Xử lý tải xuống qua pipe
            } else {
                continue;
            }
            files = "";
                } else if (command.type == MSG_TYPE_SHARE) {  // Nếu lệnh là "share"
            Message response;
            recv_message(sockfd, &response);  // Nhận phản hồi từ server
            if (response.type == MSG_TYPE_ERROR) {  // Nếu có lỗi
                printf("%s\n", response.payload);
                sprintf(log_msg, "Command output: %s", response.payload);  // Ghi log về lỗi
                log_message('e', log_msg);
                continue;
            } else {
                recv_message(sockfd, &response);  // Nhận phản hồi sau khi chia sẻ
                if (response.type == MSG_TYPE_ERROR) {  // Nếu có lỗi
                    printf("%s\n", response.payload);
                    sprintf(log_msg, "Command output: %s", response.payload);  // Ghi log về lỗi
                    log_message('e', log_msg);
                    continue;
                } else {
                    printf("Shared successfully!\n");  // Nếu chia sẻ thành công
                    sprintf(log_msg, "Share command successfully");  // Ghi log khi chia sẻ thành công
                    log_message('i', log_msg);
                }
            }

        } else if (command.type == MSG_TYPE_PWD) {  // Nếu lệnh là "pwd"
            const char *pwd = handle_pwd(cur_user, user_dir);  // Lấy đường dẫn thư mục hiện tại
            printf("%s\n", pwd);  // In ra đường dẫn thư mục hiện tại
        } else if (command.type == MSG_TYPE_UPLOAD) {  // Nếu lệnh là "upload"
            handle_upload(data_sock, command.payload, sockfd);  // Xử lý tải lên
        } else if (command.type == MSG_TYPE_RELOAD) {  // Nếu lệnh là "reload"
            // Không có hành động cụ thể cho lệnh này
        }
        close(data_sock);  // Đóng kết nối dữ liệu sau khi xử lý xong

    }   // Kết thúc vòng lặp và quay lại để người dùng nhập lệnh mới

    free(cur_user);  // Giải phóng bộ nhớ cho tên người dùng
    free(log_msg);  // Giải phóng bộ nhớ cho thông báo log
    close(sockfd);  // Đóng kết nối chính với server
    return 0;  // Kết thúc chương trình
}

int cli_read_command(char *user_input, int size, Message *msg) {
    // wait for user to enter a command
    read_input(user_input, size);

    if (strcmp(user_input, "ls ") == 0 || strcmp(user_input, "ls") == 0) {
        msg->type = MSG_TYPE_LS;
    } else if (strncmp(user_input, "cd ", 3) == 0) {
        msg->type = MSG_TYPE_CD;
        strcpy(msg->payload, user_input + 3);
    } else if (strncmp(user_input, "find ", 5) == 0) {
        std::string cmd(user_input);
        std::string left_cmd, right_cmd;
        size_t pos = cmd.find('|');
        if (pos != std::string::npos) {
            // '|' found, split the string
            left_cmd = cmd.substr(0, pos);
            right_cmd = cmd.substr(pos + 1);

            left_cmd.erase(left_cmd.find_last_not_of(" \n\r\t") + 1);
            right_cmd.erase(0, right_cmd.find_first_not_of(" \n\r\t"));
            if (!(right_cmd == "dl" || right_cmd == "download")) {
                printf("%s not supported!\n", right_cmd.c_str());
                return -1;
            } else {
                msg->type = MSG_TYPE_FIND;
                strcpy(msg->payload, user_input + 5);
            }
        } else {
            msg->type = MSG_TYPE_FIND;
            strcpy(msg->payload, user_input + 5);
        }

    } else if (strncmp(user_input, "rm", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "mv", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "cp", 2) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "share ", 6) == 0) {
        msg->type = MSG_TYPE_SHARE;
        strcpy(msg->payload, user_input + 6);
    } else if (strncmp(user_input, "mkdir", 5) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "touch", 5) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "cat", 3) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strncmp(user_input, "echo", 4) == 0) {
        msg->type = MSG_TYPE_BASIC_COMMAND;
        strcpy(msg->payload, user_input);
    } else if (strcmp(user_input, "pwd") == 0 || strcmp(user_input, "pwd ") == 0) {
        msg->type = MSG_TYPE_PWD;
    } else if (strncmp(user_input, "up ", 3) == 0 || strncmp(user_input, "upload ", 7) == 0) {
        msg->type = MSG_TYPE_UPLOAD;
        if (strncmp(user_input, "up ", 3) == 0) {
            strcpy(msg->payload, user_input + 3);
        } else {
            strcpy(msg->payload, user_input + 7);
        }
    } else if (strncmp(user_input, "dl ", 3) == 0 || strncmp(user_input, "download ", 9) == 0) {
        msg->type = MSG_TYPE_DOWNLOAD;
        if (strncmp(user_input, "dl ", 3) == 0) {
            strcpy(msg->payload, user_input + 3);
        } else {
            strcpy(msg->payload, user_input + 9);
        }
    } else if (strcmp(user_input, "quit") == 0 || strcmp(user_input, "quit ") == 0 ||
               strcmp(user_input, "exit") == 0 || strcmp(user_input, "exit ") == 0) {
        msg->type = MSG_TYPE_QUIT;
    } else if (strcmp(user_input, "clear") == 0) {
        msg->type = MSG_TYPE_CLEAR;
    } else if (strcmp(user_input, "reload") == 0) {
        msg->type = MSG_TYPE_RELOAD;
    } else if (strcmp(user_input, "help") == 0) {
        printf(ANSI_BOLD ANSI_COLOR_BLUE "ls" ANSI_RESET
                                         ": list files and folders in current directory\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "cd <path>" ANSI_RESET ": change directory to <path>\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "mkdir" ANSI_RESET ": create folder\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "touch" ANSI_RESET ": create file\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "cat" ANSI_RESET ":show file content \n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "mv" ANSI_RESET ":rename or move file/folder \n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "cp" ANSI_RESET ":copy file or folder \n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "rm" ANSI_RESET ":delete file or folder \n");

        printf(ANSI_BOLD ANSI_COLOR_BLUE "pwd" ANSI_RESET ": print working directory\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "find <pattern>" ANSI_RESET
                                         ": find files and folders in current directory\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE
               "find <pattern> | dl" ANSI_RESET
               ": find files and folders in current directory and download\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "upload <path>" ANSI_RESET
                                         ": upload file or folder to current directory\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "download <path>" ANSI_RESET
                                         ": download file or folder to current directory\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "share <path>" ANSI_RESET
                                         ": share file or folder to other users\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "quit" ANSI_RESET ": exit cppdrive\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "clear" ANSI_RESET ": clear screen\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "reload" ANSI_RESET ": reload file system\n");
        printf(ANSI_BOLD ANSI_COLOR_BLUE "help" ANSI_RESET ": show help\n");
        msg->type = MSG_TYPE_RELOAD;
    } else {
        return -1;
    }
    return 0;
}

/**
 * Sinh khóa đối xứng AES và gửi đến server
 * 1. Sinh khóa AES nếu thành công thì lưu trữ, nếu thất bại thì sử dụng khóa mặc định
 * 2. Nhận khóa công khai từ server
 * 3. Mã hóa khóa đối xứng bằng khóa công khai của server
 * 4. Gửi khóa đối xứng đã mã hóa trở lại server
 */
void handle_symmetric_key_pair(int sockfd) {
    std::string aesKey;
    if (generate_symmetric_key(aesKey)) {
        log_message('i', "AES key generated successfully!");
        log_message('i', aesKey.c_str());
    } else {
        log_message('w', "AES key generation failed, use default key");
        for (int i = 0; i < SYMMETRIC_KEY_SIZE; i++) {
            aesKey.push_back('0' + i);
        }
    }

    Message key;
    recv_message(sockfd, &key);
    std::string public_server_key(key.payload);

    log_message('i', "Received public key from server");
    log_message('i', public_server_key.c_str());

    std::string ciphertext;
    if (encrypt_symmetric_key(public_server_key, aesKey, ciphertext)) {
        Message msg;
        msg.type = MSG_DATA_PUBKEY;
        msg.length = ciphertext.size();
        memcpy(msg.payload, ciphertext.data(), ciphertext.size() + 1);
        send_message(sockfd, msg);
        log_message('i', "Send encrypted symmetric key to server");
        log_message('i', ciphertext.c_str());
    } else {
        log_message('e', "Failed to send encrypted symmetric key to server!");
        exit(1);
    }
    SYMMETRIC_KEY = aesKey;
}

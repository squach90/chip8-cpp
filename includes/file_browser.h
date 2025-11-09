#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

// == Raw mode to get the keys ==
static void set_raw_mode(int enable) {
    static struct termios oldt;
    struct termios newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

// == Read keys ==
static char *get_key(void) {
    static char buffer[4];
    memset(buffer, 0, sizeof(buffer));
    read(STDIN_FILENO, buffer, 1);
    if (buffer[0] == '\x1b') {
        read(STDIN_FILENO, buffer + 1, 2);
    }
    return buffer;
}

// == File Browser==
static char* select_file(void) {
    char current_path[PATH_MAX];
    getcwd(current_path, sizeof(current_path));
    int selected = 0;

    while (1) {
        DIR *dir = opendir(current_path);
        if (!dir) {
            perror("opendir");
            return NULL;
        }

        struct dirent *entry;
        char items[256][NAME_MAX];
        int is_dir[256];
        int count = 0;

        while ((entry = readdir(dir)) != NULL && count < 256) {
            if (strcmp(entry->d_name, ".") == 0) continue;
            strcpy(items[count], entry->d_name);
            struct stat st;
            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", current_path, entry->d_name);
            stat(fullpath, &st);
            is_dir[count++] = S_ISDIR(st.st_mode);
        }
        closedir(dir);

        system("clear");
        printf("📂 File Browser\n"); // == TITLE ==
        printf("Path: %s\n", current_path);
        printf("↑/↓ Navigate | Enter = Open\n");
        printf("----------------------------------------\n");

        for (int i = 0; i < count; i++) {
            printf("%s %s%s\n",
                   (i == selected ? "> " : "  "),
                   (is_dir[i] ? "[DIR] " : "[FILE] "),
                   items[i]);
        }

        set_raw_mode(1);
        char *key = get_key();
        set_raw_mode(0);

        if (strcmp(key, "\x1b[A") == 0) {
            selected = (selected - 1 + count) % count;
        } else if (strcmp(key, "\x1b[B") == 0) {
            selected = (selected + 1) % count;
        } else if (key[0] == '\n' || key[0] == '\r') {
            if (is_dir[selected]) {
                if (strcmp(items[selected], "..") == 0) {
                    char *slash = strrchr(current_path, '/');
                    if (slash && slash != current_path)
                        *slash = '\0';
                    else
                        strcpy(current_path, "/");
                } else {
                    strcat(current_path, "/");
                    strcat(current_path, items[selected]);
                }
                selected = 0;
            } else {
                char *result = malloc(PATH_MAX);
                snprintf(result, PATH_MAX, "%s/%s", current_path, items[selected]);
                return result;
            }
        }
    }
}

#endif

//
// Created by zkw on 20-12-25.
//

#include <string>
#include <unistd.h>
#include <sys/stat.h>

#include "common_utils.h"

using namespace std;

static char *packageName = nullptr;

string getPackageName() {
    if (packageName != nullptr) {
        return packageName;
    } else {
        pid_t pid = getpid();
        string cmdlinePath = "/proc/" + to_string(pid) + "/cmdline";
        FILE *cmdline = fopen(cmdlinePath.c_str(), "r");
        if (cmdline) {
            uint16_t size = 128;
            packageName = static_cast<char *>(malloc(size));
            fread(packageName, size, 1, cmdline);
            fclose(cmdline);
            return packageName;
        } else {
            return "";
        }
    }
}

void replaceAll(string &str, const string &from, const string &to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

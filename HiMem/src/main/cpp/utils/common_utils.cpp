//
// Created by zkw on 20-12-25.
//

#include <string>
#include <unistd.h>

#include "common_utils.h"

using namespace std;

string getPackageName() {
    pid_t pid = getpid();
    string cmdlinePath = "/proc/" + to_string(pid) + "/cmdline";
    FILE *cmdline = fopen(cmdlinePath.c_str(), "r");
    if (cmdline) {
        char packageName[64] = {0};
        fread(packageName, sizeof(packageName), 1, cmdline);
        fclose(cmdline);
        return packageName;
    } else {
        return "";
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

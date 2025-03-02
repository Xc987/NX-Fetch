#include <stdio.h>
#include <string.h>
#include <switch.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_DEPTH 2
#define CONTENT_PATH "/atmosphere/contents/"
#define MAX_PATH_LEN 512

int countNroFiles(const char *directory, int depth) {
    if (depth > MAX_DEPTH) return 0;
    int count = 0;
    DIR *dir = opendir(directory);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { 
            if (strstr(entry->d_name, ".nro")) {
                count++;
            }
        } else if (entry->d_type == DT_DIR && depth < MAX_DEPTH) { 
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char newPath[PATH_MAX];
                snprintf(newPath, sizeof(newPath), "%s/%s", directory, entry->d_name);
                count += countNroFiles(newPath, depth + 1);
            }
        }
    }
    closedir(dir);
    return count;
}
int countOvlFiles(const char *path) {
    DIR *dir = opendir(path);
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".ovl") == 0) {
                count++;
            }
        }
    }
    closedir(dir);
    return count;
}

int countFoldersWithFlagAndExefsNsp() {
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    if ((dir = opendir(CONTENT_PATH)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char folderPath[MAX_PATH_LEN];
            char flagPath[MAX_PATH_LEN];
            char exefsNspPath[MAX_PATH_LEN];
            if (strlen(CONTENT_PATH) + strlen(ent->d_name) >= MAX_PATH_LEN) {
                continue; 
            }
            strcpy(folderPath, CONTENT_PATH);
            strcat(folderPath, ent->d_name);
            struct stat st;
            if (stat(folderPath, &st) == 0 && S_ISDIR(st.st_mode)) {
                if (strlen(folderPath) + strlen("/flags") >= MAX_PATH_LEN) {
                    printf("Path too long: %s/flags\n", folderPath);
                    continue;
                }
                strcpy(flagPath, folderPath);
                strcat(flagPath, "/flags");
                if (strlen(folderPath) + strlen("/exefs.nsp") >= MAX_PATH_LEN) {
                    printf("Path too long: %s/exefs.nsp\n", folderPath);
                    continue;
                }
                strcpy(exefsNspPath, folderPath);
                strcat(exefsNspPath, "/exefs.nsp");
                if (stat(flagPath, &st) == 0 && S_ISDIR(st.st_mode) &&
                    stat(exefsNspPath, &st) == 0 && S_ISREG(st.st_mode)) {
                    count++;
                }
            }
        }
        closedir(dir);
    }
    return count;
}

int main(int argc, char **argv) {
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    printf("\n\e[38;5;33m   ######## \e[38;5;196m#######\n");
    printf("\e[38;5;33m  ##      # \e[38;5;196m#########\n");
    printf("\e[38;5;33m #        # \e[38;5;196m##########\n");
    printf("\e[38;5;33m#   ###   # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#  #####  # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#  #####  # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#  #####  # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#   ###   # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#         # \e[38;5;196m####   ####\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###     ###\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###     ###\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###     ###\n");
    printf("\e[38;5;33m#         # \e[38;5;196m####   ####\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###########\n");
    printf("\e[38;5;33m#         # \e[38;5;196m###########\n");
    printf("\e[38;5;33m #        # \e[38;5;196m########## \n");
    printf("\e[38;5;33m  ##      # \e[38;5;196m#########\n");
    printf("\e[38;5;33m   ######## \e[38;5;196m#######\n");

    // Variables
    bool userflag = false;

    // Console nickname
    setsysInitialize();
    SetSysDeviceNickName nickname;
    memset(&nickname, 0, sizeof(nickname));
    setsysGetDeviceNickname(&nickname);
    setsysExit();

    // Preselected user
    Result rc = accountInitialize(AccountServiceType_Application);
    AccountProfileBase profileBase;
    if (R_SUCCEEDED(rc)) {
        AccountUid userID;
        rc = accountGetPreselectedUser(&userID);
        if (R_SUCCEEDED(rc)) {
            AccountProfile profile;
            rc = accountGetProfile(&profile, userID);
            if (R_SUCCEEDED(rc)) {
                rc = accountProfileGet(&profile, NULL, &profileBase);
                userflag = true;
                accountProfileClose(&profile);
            }
        }
    }
    accountExit();

    // HOS version
    setsysInitialize();
    SetSysFirmwareVersion fw_version;
    setsysGetFirmwareVersion(&fw_version);
    setsysExit();

    // AMS version
    char versionString[64];
    u64 version;
    splInitialize();
    if (R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &version))) {
        int major = (version >> 56) & 0xFF;
        int minor = (version >> 48) & 0xFF;
        int patch = (version >> 40) & 0xFF;
        snprintf(versionString, sizeof(versionString), "%d.%d.%d", major, minor, patch);
        if (R_SUCCEEDED(splGetConfig((SplConfigItem)65007, &version))) {
            strncat(versionString, version ? "|E" : "|S", sizeof(versionString) - strlen(versionString) - 1);
        }
    } else {
        snprintf(versionString, sizeof(versionString), "Unknown");
    }
    splExit();

    // Display info
    int len = strlen(nickname.nickname);
    int len2 = strlen(profileBase.nickname);
    printf(CONSOLE_ESC(0m)CONSOLE_ESC(2;27H));
    if (userflag) {
        printf("%s@%s", profileBase.nickname, nickname.nickname);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < (len + len2) + 1; i++) {
            printf("-");
        }
    } else {
        printf("%s", nickname.nickname);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < len ; i++) {
            printf("-");
        }
    }
    printf(CONSOLE_ESC(4;27H));
    printf("OS: Horizon OS %d.%d.%d", fw_version.major, fw_version.minor, fw_version.micro);

    printf(CONSOLE_ESC(5;27H));
    printf("CFW: Atmosphere %s", versionString);

    printf(CONSOLE_ESC(6;27H));
    FILE *file = fopen("/bootloader/hekate_ipl.ini", "r");
    if (file){
        printf("Bootloader: Hekate IPL");
        fclose(file);
    } else {
        printf("Bootloader: Unknown");
    }
    printf(CONSOLE_ESC(8;27H));
    int nroCount = countNroFiles("/switch", 0);
    int ovlCount = countOvlFiles("/switch/.overlays/");
    int sysCount = countFoldersWithFlagAndExefsNsp();
    printf("Packages: %d (nro), %d (ovl), %d (sys)", nroCount, ovlCount, sysCount);

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        u64 uptime_ticks = armGetSystemTick();
        u64 uptime_seconds = uptime_ticks / 19200000;
        printf(CONSOLE_ESC(7;27H));
        if (uptime_seconds < 60) {
            printf("Uptime: %lu seconds\n", uptime_seconds);
        } else if (uptime_seconds < 3600) {
            u64 uptime_minutes = uptime_seconds / 60;
            u64 remaining_seconds = uptime_seconds % 60;
            printf("Uptime: %lu minutes, %lu seconds\n", uptime_minutes, remaining_seconds);
        } else if (uptime_seconds < 86400) {
            u64 uptime_hours = uptime_seconds / 3600;
            u64 remaining_minutes = (uptime_seconds % 3600) / 60;
            printf("Uptime: %lu hours, %lu minutes\n", uptime_hours, remaining_minutes);
        } else {
            u64 uptime_days = uptime_seconds / 86400;
            u64 remaining_hours = (uptime_seconds % 86400) / 3600;
            printf("Uptime: %lu days, %lu hours\n", uptime_days, remaining_hours);
        }
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
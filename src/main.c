#include <stdio.h>
#include <string.h>
#include <switch.h>
#include <dirent.h>
#include <sys/stat.h>

bool both = false;
bool userflag = false;
int selected = 1;

u32 GetClock(PcvModule module) {
    u32 out = 0;
    if (hosversionAtLeast(8, 0, 0)) {
        clkrstInitialize();
        ClkrstSession session = {0};
        PcvModuleId module_id;
        pcvGetModuleId(&module_id, module);
        clkrstOpenSession(&session, module_id, 3);
        clkrstGetClockRate(&session, &out);
        clkrstCloseSession(&session);
        clkrstExit();
    }
    return out / 1000000;
}

int countNroFiles(const char *directory, int depth) {
    if (depth > 2) return 0;
    int count = 0;
    DIR *dir = opendir(directory);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { 
            if (strstr(entry->d_name, ".nro")) {
                count++;
            }
        } else if (entry->d_type == DT_DIR && depth < 2) { 
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

int countSysFolders() {
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    if ((dir = opendir("/atmosphere/contents/")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char folderPath[512];
            char flagPath[512];
            char exefsNspPath[512];
            if (strlen("/atmosphere/contents/") + strlen(ent->d_name) >= 512) {
                continue; 
            }
            strcpy(folderPath, "/atmosphere/contents/");
            strcat(folderPath, ent->d_name);
            struct stat st;
            if (stat(folderPath, &st) == 0 && S_ISDIR(st.st_mode)) {
                if (strlen(folderPath) + strlen("/flags") >= 512) {
                    continue;
                }
                strcpy(flagPath, folderPath);
                strcat(flagPath, "/flags");
                if (strlen(folderPath) + strlen("/exefs.nsp") >= 512) {
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
void printAscii(char* c1, char* c2) {
    printf("\n");
    printf("%s%s%s%s", c1, "    ####### ", c2, "#######\n");
    printf("%s%s%s%s", c1, "  ##      # ", c2, "#########\n");
    printf("%s%s%s%s", c1, " #        # ", c2, "##########\n");
    printf("%s%s%s%s", c1, "#   ###   # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#  #####  # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#  #####  # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#  #####  # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#   ###   # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "####   ####\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###     ###\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###     ###\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###     ###\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "####   ####\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, "#         # ", c2, "###########\n");
    printf("%s%s%s%s", c1, " #        # ", c2, "########## \n");
    printf("%s%s%s%s", c1, "  ##      # ", c2, "#########\n");
    printf("%s%s%s%s", c1, "    ####### ", c2, "#######\n");
}
void callPrintAscii() {
    if (selected == 1){
        printAscii("\e[38;5;33m", "\e[38;5;196m");
    } else if (selected == 2) {
        printAscii("\e[38;5;242m", "\e[38;5;242m");
    } else if (selected == 3) {
        printAscii("\e[38;5;254m", "\e[38;5;254m");
    } else if (selected == 4) {
        printAscii("\e[38;5;157m", "\e[38;5;159m");
    } else if (selected == 5) {
        printAscii("\e[38;5;21m", "\e[38;5;227m");
    } else if (selected == 6) {
        printAscii("\e[38;5;172m", "\e[38;5;227m");
    } else if (selected == 7) {
        printAscii("\e[38;5;27m", "\e[38;5;220m");
    } else if (selected == 8) {
        printAscii("\e[38;5;220m", "\e[38;5;27m");
    } else if (selected == 9) {
        printAscii("\e[38;5;27m", "\e[38;5;57m");
    } else if (selected == 10) {
        printAscii("\e[38;5;196m", "\e[38;5;196m");
    } else if (selected == 11) {
        printAscii("\e[38;5;33m", "\e[38;5;33m");
    } else if (selected == 12) {
        printAscii("\e[38;5;40m", "\e[38;5;40m");
    } else if (selected == 13) {
        printAscii("\e[38;5;198m", "\e[38;5;198m");
    } else if (selected == 14) {
        printAscii("\e[38;5;129m", "\e[38;5;220m");
    } else if (selected == 15) {
        printAscii("\e[38;5;227m", "\e[38;5;227m");
    } else if (selected == 16) {
        printAscii("\e[38;5;225m", "\e[38;5;120m");
    } else if (selected == 17) {
        printAscii("\e[38;5;198m", "\e[38;5;40m");
    }
}
void printConfig(const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", "/bootloader/ini/", filename);
    FILE *file = fopen(filepath, "r");
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *start = strchr(line, '[');
        char *end = strchr(line, ']');
        if (start && end && end > start + 1) {
            *end = '\0';
            if (both) {
                printf(", ");
            }
            printf("%s",start + 1);
            break;
        }
    }
    fclose(file);
}
int main(int argc, char **argv) {
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    printAscii("\e[38;5;33m", "\e[38;5;196m");
    consoleUpdate(NULL);

    // Device nickname
    setsysInitialize();
    SetSysDeviceNickName nickname;
    memset(&nickname, 0, sizeof(nickname));
    setsysGetDeviceNickname(&nickname);
    setsysExit();
    // Pre selected user
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
    int len = strlen(nickname.nickname);
    int len2 = strlen(profileBase.nickname);
    printf(CONSOLE_ESC(0m)CONSOLE_ESC(2;27H));
    if (userflag) {
        printf("\e[38;5;33m%s\e[38;5;255m@\e[38;5;33m%s\e[38;5;255m", profileBase.nickname, nickname.nickname);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < (len + len2) + 1; i++) {
            printf("-");
        }
    } else {
        printf("\e[38;5;33m%s\e[38;5;255m", nickname.nickname);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < len ; i++) {
            printf("-");
        }
    }
    consoleUpdate(NULL);
    // Horizon OS firmware version
    setsysInitialize();
    SetSysFirmwareVersion fw_version;
    setsysGetFirmwareVersion(&fw_version);
    setsysExit();
    printf(CONSOLE_ESC(4;27H));
    printf("\e[38;5;33mOS\e[38;5;255m: Horizon OS %d.%d.%d", fw_version.major, fw_version.minor, fw_version.micro);
    consoleUpdate(NULL);
    // Atmosphere version
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
    printf(CONSOLE_ESC(5;27H));
    printf("\e[38;5;33mCFW\e[38;5;255m: Atmosphere %s", versionString);
    consoleUpdate(NULL);
    // Bootloader
    printf(CONSOLE_ESC(6;27H));
    FILE *file = fopen("/bootloader/hekate_ipl.ini", "r");
    if (file){
        printf("\e[38;5;33mBL\e[38;5;255m: Hekate IPL");
        fclose(file);
    } else {
        printf("\e[38;5;33mBL\e[38;5;255m: Unknown");
    }
    consoleUpdate(NULL);
    // Bootloader configs
    DIR *dir = opendir("/bootloader/ini/");
    
    printf(CONSOLE_ESC(7;27H));
    printf("\e[38;5;33mBL configs\e[38;5;255m: ");
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, ".ini")) {
            printConfig(entry->d_name);
            both = true;
        }
    }
    if (!both) {
        printf("None");
    }
    closedir(dir);
    // System uptime
    u64 uptime_ticks = armGetSystemTick();
    u64 uptime_seconds = uptime_ticks / 19200000;
    printf(CONSOLE_ESC(8;27H));
    if (uptime_seconds < 60) {
        printf("\e[38;5;33mUptime\e[38;5;255m: %lu seconds\n", uptime_seconds);
    } else if (uptime_seconds < 3600) {
        u64 uptime_minutes = uptime_seconds / 60;
        u64 remaining_seconds = uptime_seconds % 60;
        printf("\e[38;5;33mUptime\e[38;5;255m: %lu minutes, %lu seconds\n", uptime_minutes, remaining_seconds);
    } else if (uptime_seconds < 86400) {
        u64 uptime_hours = uptime_seconds / 3600;
        u64 remaining_minutes = (uptime_seconds % 3600) / 60;
        printf("\e[38;5;33mUptime\e[38;5;255m: %lu hours, %lu minutes\n", uptime_hours, remaining_minutes);
    } else {
        u64 uptime_days = uptime_seconds / 86400;
        u64 remaining_hours = (uptime_seconds % 86400) / 3600;
        printf("\e[38;5;33mUptime\e[38;5;255m: %lu days, %lu hours\n", uptime_days, remaining_hours);
    }
    // Packages
    int nroCount = countNroFiles("/switch", 0);
    int ovlCount = countOvlFiles("/switch/.overlays/");
    int sysCount = countSysFolders();
    printf(CONSOLE_ESC(9;27H));
    printf("\e[38;5;33mPackages\e[38;5;255m: %d (nro), %d (ovl), %d (sys)", nroCount, ovlCount, sysCount);
    consoleUpdate(NULL);
    // Screen resolution
    u32 width, height;
    NWindow* nw = nwindowGetDefault();
    nwindowGetDimensions(nw, &width, &height);
    printf(CONSOLE_ESC(10;27H));
    printf("\e[38;5;33mResolution\e[38;5;255m: %d x %d ", width, height);
    consoleUpdate(NULL);
    // CPU
    printf(CONSOLE_ESC(11;27H));
    printf("\e[38;5;33mCPU\e[38;5;255m: ARM 4 Cortex-A57 (4) @ %u MHz",GetClock(PcvModule_CpuBus));
    consoleUpdate(NULL);
    // GPU
    printf(CONSOLE_ESC(12;27H));
    printf("\e[38;5;33mGPU\e[38;5;255m: Nvidia GM20B @ %u MHz",GetClock(PcvModule_GPU));
    consoleUpdate(NULL);
    // Memory
    u64 totalRam = 0;
    u64 usedRam = 0;
    svcGetInfo(&totalRam, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
    svcGetInfo(&usedRam, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
    printf(CONSOLE_ESC(13;27H));
    printf("\e[38;5;33mMemory\e[38;5;255m: %ldMB / %ldMB @ %u MHz", (usedRam / 1024 / 1024), (totalRam / 1024 / 1024), GetClock(PcvModule_EMC));
    consoleUpdate(NULL);
    // SD card
    FsFileSystem *sdFs = fsdevGetDeviceFileSystem("sdmc");
    s64 totalSpaceBytes, freeSpaceBytes;
    fsFsGetTotalSpace(sdFs, "/", &totalSpaceBytes);
    fsFsGetFreeSpace(sdFs, "/", &freeSpaceBytes);
    double totalSpaceGB = (double)totalSpaceBytes / (1024 * 1024 * 1024);
    double freeSpaceGB = (double)freeSpaceBytes / (1024 * 1024 * 1024);
    double leftSpaceGB = totalSpaceGB - freeSpaceGB;
    printf(CONSOLE_ESC(14;27H));
    printf("\e[38;5;33mDisk (SD)\e[38;5;255m: %.2fGB / %.2fGB ", leftSpaceGB, totalSpaceGB);
    consoleUpdate(NULL);
    // NAND
    fsInitialize();
    FsFileSystem userFs;
    fsOpenBisFileSystem(&userFs, FsBisPartitionId_User, "");
    s64 totalSpaceBytes2, freeSpaceBytes2;
    fsFsGetTotalSpace(&userFs, "/", &totalSpaceBytes2);
    fsFsGetFreeSpace(&userFs, "/", &freeSpaceBytes2);
    double totalSpaceGB2 = (double)totalSpaceBytes2 / (1024 * 1024 * 1024);
    double freeSpaceGB2 = (double)freeSpaceBytes2 / (1024 * 1024 * 1024);
    double leftSpaceGB2 = totalSpaceGB2 - freeSpaceGB2;
    fsFsClose(&userFs);
    fsExit();
    printf(CONSOLE_ESC(15;27H));
    printf("\e[38;5;33mDisk (NAND)\e[38;5;255m: %.2fGB / %.2fGB ", leftSpaceGB2, totalSpaceGB2);
    consoleUpdate(NULL);
    // Battery
    psmInitialize();
    u32 batteryCharge;
    psmGetBatteryChargePercentage(&batteryCharge);
    printf(CONSOLE_ESC(16;27H));
    printf("\e[38;5;33mBattery\e[38;5;255m: %d%%", batteryCharge);
    PsmChargerType chargerType;
    psmGetChargerType(&chargerType);
    if (chargerType != PsmChargerType_Unconnected){
        printf(" [Charging]");
    } else {
        printf(" [Discharging]");
    }
    psmExit();
    // Fancy color blocks
    printf(CONSOLE_ESC(18;27H));
    printf("\e[48;5;235m   \e[48;5;1m   \e[48;5;2m   \e[48;5;3m   \e[48;5;4m   \e[48;5;5m   \e[48;5;6m   \e[48;5;7m   \e[48;5;0m");
    printf(CONSOLE_ESC(19;27H));
    printf("\e[48;5;8m   \e[48;5;9m   \e[48;5;10m   \e[48;5;11m   \e[48;5;12m   \e[48;5;13m   \e[48;5;14m   \e[48;5;15m   \e[48;5;0m");
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        if (kDown & HidNpadButton_AnyRight) {
            if (selected != 17) {
                selected = selected + 1;
                printf(CONSOLE_ESC(1;1H));
                callPrintAscii();
            }
        }
        if (kDown & HidNpadButton_AnyLeft) {
            if (selected != 1) {
                selected = selected - 1;
                printf(CONSOLE_ESC(1;1H));
                callPrintAscii();
            }
        }
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
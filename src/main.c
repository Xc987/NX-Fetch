#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

bool both = false;
bool userflag = false;
bool anycontroller = false;
int selected = 1;
char* userName;
char* deviceName;
char userNames[10][50];
AccountUid userAccounts[10];
int selectedUser = 0;
s32 totalUsers = 0;
char leftColors[20][15] = {
    "\e[38;5;33m", "\e[38;5;242m", "\e[38;5;254m","\e[38;5;157m",
    "\e[38;5;21m", "\e[38;5;172m", "\e[38;5;27m","\e[38;5;220m",
    "\e[38;5;27m", "\e[38;5;196m", "\e[38;5;33m","\e[38;5;40m",
    "\e[38;5;198m", "\e[38;5;129m", "\e[38;5;227m", 
    "\e[38;5;225m", "\e[38;5;198m"
};
char rightColors[20][15] = {
    "\e[38;5;196m", "\e[38;5;242m", "\e[38;5;254m","\e[38;5;159m",
    "\e[38;5;227m", "\e[38;5;227m", "\e[38;5;220m","\e[38;5;27m",
    "\e[38;5;57m", "\e[38;5;196m", "\e[38;5;33m","\e[38;5;40m",
    "\e[38;5;198m", "\e[38;5;220m", "\e[38;5;227m", 
    "\e[38;5;120m", "\e[38;5;40m"
};
char infoLabels[20][20] = {
    "OS", "CFW", "BL", "BL Configs", "Uptime","Packages", 
    "Display", "Theme", "CPU", "GPU", "Memory", "Hardware", "Disk (sdmc:/)", 
    "Disk (user:/)", "Disk (system:/)","Battery", "Local IP", 
    "Locale", "Controllers"
};
bool isVersionString(const char* str, size_t len) {
    if (len < 5) return false;
    int dot_count = 0;
    int digit_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '.') {
            dot_count++;
            if (i == 0 || i == len-1 || str[i+1] == '.') {
                return false;
            }
        } else if (str[i] >= '0' && str[i] <= '9') {
            digit_count++;
        } else {
            return false;
        }
    }
    return (dot_count == 2) && (digit_count >= 3);
}
void printBootloaderVersion(u8* data, size_t size) {
    for (size_t i = 0; i < size - 5; i++) {
        size_t max_search = (i + 32 < size) ? 32 : size - i;
        for (size_t len = 5; len <= max_search; len++) {
            char* candidate = (char*)(data + i);
            if (isVersionString(candidate, len)) {
                char temp = candidate[len];
                candidate[len] = '\0';
                if (candidate[0] >= '3' && candidate[0] <= '6') {
                    printf("%s ", candidate);
                }
                candidate[len] = temp;
                i += len - 1;
                break;
            }
        }
    }
}
const char* getRegionName(u64 region) {
    switch (region) {
        case SetRegion_JPN: return "JP";
        case SetRegion_USA: return "US";
        case SetRegion_EUR: return "EU";
        case SetRegion_AUS: return "AU";
        case SetRegion_HTK: return "HK";
        case SetRegion_CHN: return "CN";
        default: return "Unknown";
    }
}
const char* GetLanguageName(u64 languageCode) {
    switch (languageCode) {
        case SetLanguage_JA: return "jp";
        case SetLanguage_ENUS: return "en";
        case SetLanguage_ENGB: return "uk";
        case SetLanguage_FR: return "fr";
        case SetLanguage_DE: return "de";
        case SetLanguage_IT: return "it";
        case SetLanguage_ES: return "es";
        case SetLanguage_ZHCN: return "cn";
        case SetLanguage_KO: return "kr";
        case SetLanguage_NL: return "nl";
        case SetLanguage_PT: return "pt";
        case SetLanguage_RU: return "ru";
        case SetLanguage_ZHTW: return "cn";
        case SetLanguage_FRCA: return "fr";
        case SetLanguage_ES419: return "es";
        case SetLanguage_ZHHANS: return "cn";
        case SetLanguage_ZHHANT: return "cn";
        default: return "Unknown";
    }
}
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
int countPackages(const char *directory, int depth, char *extension) {
    if (depth > 2) return 0;
    int count = 0;
    DIR *dir = opendir(directory);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { 
            if (strstr(entry->d_name, extension)) {
                count++;
            }
        } else if (entry->d_type == DT_DIR && depth < 2) { 
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char newPath[PATH_MAX];
                snprintf(newPath, sizeof(newPath), "%s/%s", directory, entry->d_name);
                count += countPackages(newPath, depth + 1, extension);
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
void printHidBatteryProcentage(HidPowerInfo info) {
    if (info.battery_level == 4) {
        printf("\e[38;5;40m100%%\e[38;5;255m, ");
    } else if (info.battery_level == 3) {
        printf("\e[38;5;148m75%%\e[38;5;255m, ");
    } else if (info.battery_level == 2) {
        printf("\e[38;5;226m50%%\e[38;5;255m, ");
    } else if (info.battery_level == 1) {
        printf("\e[38;5;208m25%%\e[38;5;255m, ");
    } else if (info.battery_level == 0) {
        printf("\e[38;5;196m0%%\e[38;5;255m, ");
    }
}
void printHidBattery(HidNpadIdType hidDevice) {
    HidPowerInfo info;
    hidGetNpadPowerInfoSingle(hidDevice, &info);
    printHidBatteryProcentage(info);
}
void printHidBatteryL(HidNpadIdType hidDevice) {
    HidPowerInfo info_left;
    HidPowerInfo info_right;
    hidGetNpadPowerInfoSplit(hidDevice, &info_left, &info_right);
    printHidBatteryProcentage(info_left);
}
void printHidBatteryR(HidNpadIdType hidDevice) {
    HidPowerInfo info_left;
    HidPowerInfo info_right;
    hidGetNpadPowerInfoSplit(hidDevice, &info_left, &info_right);
    printHidBatteryProcentage(info_right);
}
void printController(u32 device_type, const char* controller_name, HidNpadIdType hidDevice) {
    int both = 0;
    if (device_type == 0) {
        return;
    }
    anycontroller = true;
    printf("%s: ", controller_name);
    if (device_type & HidDeviceTypeBits_FullKey) {
        printf("Pro Controller ");
        printHidBattery(hidDevice);
    }
    if (device_type & HidDeviceTypeBits_DebugPad) {
        printf("DebugPad ");
        printHidBattery(hidDevice);
    }
    if (device_type & HidDeviceTypeBits_HandheldLeft) {
        printf("Joy-Con L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_HandheldRight) {
        printf("Joy-Con R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_JoyLeft) {
        printf("Joy-Con L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_JoyRight) {
        printf("Joy-Con R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }        
    if (device_type & HidDeviceTypeBits_Palma) {
        printf("Poké Ball Plus ");
    }
    if (device_type & HidDeviceTypeBits_LarkHvcLeft) {
        printf("Famicom L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_LarkHvcRight) {
        printf("Famicom R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_LarkNesLeft) {
        printf("NES L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_LarkNesRight) {
        printf("NES R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_HandheldLarkHvcLeft) {
        printf("Famicom L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_HandheldLarkHvcRight) {
        printf("Famicom R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_HandheldLarkNesLeft) {
        printf("NES L ");
        printHidBatteryL(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_HandheldLarkNesRight) {
        printf("NES R ");
        printHidBatteryR(hidDevice);
        both += 1;
    }
    if (device_type & HidDeviceTypeBits_Lucia) {
        printf("SNES controller ");
        printHidBattery(hidDevice);
    }
    if (device_type & HidDeviceTypeBits_Lagon) {
        printf("N64 controller ");
        printHidBattery(hidDevice);
    }
    if (device_type & HidDeviceTypeBits_Lager) {
        printf("Sega Genesis controller ");
        printHidBattery(hidDevice);
    }
    if (device_type & HidDeviceTypeBits_System) {
        printf("Generic controller ");
        printHidBattery(hidDevice);
    }
    unsigned char ch;
    hidGetNpadInterfaceType(hidDevice, &ch);
    if (ch == 1) {
        printf("\b\b (BT)");
        if (both == 1){
            printf("  ");
        }
    } else if (ch == 3) {
        printf("\b\b (USB)");
        if (both == 1){
            printf("  ");
        }
    } else {
        printf("\b\b");
        both += 1;
    }
    if (both == 1){
        printf("\b\b");
    }
    printf("\n");
    printf(CONSOLE_ESC(39C));
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
void updateAscii() {
    char *accentColor = leftColors[selected - 1];
    printAscii(leftColors[selected - 1], rightColors[selected - 1]);
    printf(CONSOLE_ESC(0m)CONSOLE_ESC(2;27H));
    printf("                                                  ");
    printf(CONSOLE_ESC(0m)CONSOLE_ESC(2;27H));
    printf("%s%s\e[38;5;255m@%s%s\e[38;5;255m", accentColor, userNames[selectedUser], accentColor, deviceName);
    printf(CONSOLE_ESC(3;27H));
    printf("                                                  ");
    printf(CONSOLE_ESC(3;27H));
    for (int i = 0; i < (strlen(userNames[selectedUser]) + strlen(deviceName)) + 1; i++) {
        printf("-");
    }
    printf(CONSOLE_ESC(4;27H));
    for (int i = 0; i < 19; i++) {
        printf("%s%s\e[38;5;255m\n", accentColor, infoLabels[i]);
        printf(CONSOLE_ESC(26C));
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
u32 listTitles() {
    NsApplicationRecord *records = malloc(sizeof(NsApplicationRecord) * 256);
    int32_t recordCount = 0;
    nsListApplicationRecord(records, 256, 0, &recordCount);
    free(records);
    return recordCount;
}
int main(int argc, char **argv) {
    consoleInit(NULL);
    padConfigureInput(8, HidNpadStyleSet_NpadStandard);
    PadState pads[8];
    for (int i = 0; i < 8; i++) {
        if (i == 0) {
            padInitializeDefault(&pads[i]);
        } else {
            padInitialize(&pads[i], i);
        }
    }
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
    if (userflag == false) {
        rc = accountInitialize(AccountServiceType_System);
        if (R_FAILED(rc)) {
            accountExit();
        }
        rc = accountGetUserCount(&totalUsers);
        if (R_FAILED(rc)) {
            accountExit();
        }
        AccountUid *user_ids = (AccountUid *)malloc(sizeof(AccountUid) * totalUsers);
        s32 actual_users = 0;
        rc = accountListAllUsers(user_ids, totalUsers, &actual_users);
        if (R_FAILED(rc)) {
            free(user_ids);
            accountExit();
        }
        AccountProfile profile;
        AccountProfileBase profile_base;
        for (s32 i = 0; i < actual_users; i++) {
            userAccounts[i] = user_ids[i];
            rc = accountGetProfile(&profile, user_ids[i]);
            if (R_FAILED(rc)) {
                continue;
            }
            rc = accountProfileGet(&profile, NULL, &profile_base);
            if (R_FAILED(rc)) {
                accountProfileClose(&profile);
                continue;
            }
            strcpy(userNames[i], profile_base.nickname);
            accountProfileClose(&profile);
        }
        free(user_ids);
        accountExit();
    }
    deviceName = nickname.nickname;
    printf(CONSOLE_ESC(0m)CONSOLE_ESC(2;27H));
    if (userflag) {
        userName = profileBase.nickname;
        printf("\e[38;5;33m%s\e[38;5;255m@\e[38;5;33m%s\e[38;5;255m", userName, deviceName);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < (strlen(userName) + strlen(deviceName)) + 1; i++) {
            printf("-");
        }
    } else {
        printf("\e[38;5;33m%s\e[38;5;255m@\e[38;5;33m%s\e[38;5;255m", userNames[selectedUser], deviceName);
        printf(CONSOLE_ESC(3;27H));
        for (int i = 0; i < (strlen(userNames[selectedUser]) + strlen(deviceName)) + 1; i++) {
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
    if (file) {
        printf("\e[38;5;33mBL\e[38;5;255m: Hekate IPL ");
        fclose(file);    
        FILE* file = fopen("sdmc:/payload.bin", "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            if (file_size < 1 * 1024 * 1024) {
                u8* file_data = malloc(file_size);
                if (file_data) {
                    fread(file_data, 1, file_size, file);
                    fclose(file);
                    printBootloaderVersion(file_data, file_size);
                    free(file_data);
                }
            }
        }
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
    printf(CONSOLE_ESC(1;1H));
    printAscii(leftColors[selected - 1], rightColors[selected - 1]);
    closedir(dir);
    consoleUpdate(NULL);
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
    consoleUpdate(NULL);
    // Packages
    int nroCount = countPackages("/switch", 0, ".nro");
    int ovlCount = countPackages("/switch/.overlays/", 0, ".ovl");
    int sysCount = countSysFolders();
    nsInitialize();
    u32 titleCount = listTitles();
    nsExit();
    printf(CONSOLE_ESC(9;27H));
    printf("\e[38;5;33mPackages\e[38;5;255m: %d Titles, %d (nro), %d (ovl), %d (sys)", titleCount, nroCount, ovlCount, sysCount);
    consoleUpdate(NULL);
    // Screen resolution
    AppletOperationMode mode = appletGetOperationMode();        
    if (mode == AppletOperationMode_Handheld) {
        u32 width, height;
        NWindow* nw = nwindowGetDefault();
        nwindowGetDimensions(nw, &width, &height);
        printf(CONSOLE_ESC(10;27H));
        printf("\e[38;5;33mDisplay\e[38;5;255m: %dx%d @ 60Hz [Handheld]", width, height);
    } else if (mode == AppletOperationMode_Console) {
        s32 width = 0, height = 0;
        appletGetDefaultDisplayResolution(&width, &height);
        printf(CONSOLE_ESC(10;27H));
        printf("\e[38;5;33mDisplay\e[38;5;255m: %dx%d @ 60Hz [Docked]", width, height);
    }
    consoleUpdate(NULL);
    // Theme
    setsysInitialize();
    ColorSetId colorSet;
    rc = setsysGetColorSetId(&colorSet);
    printf(CONSOLE_ESC(11;27H));
    if (colorSet == 1) {
        printf("\e[38;5;33mTheme\e[38;5;255m: Dark mode, ");
    } else if (colorSet == 0) {
        printf("\e[38;5;33mTheme\e[38;5;255m: Light mode, ");
    }
    dir = opendir("/atmosphere/contents/0100000000001000/romfs/lyt");
    if (dir) {
        closedir(dir);
        int szsFound = 0;
        struct dirent* entry;
        dir = opendir("/atmosphere/contents/0100000000001000/romfs/lyt");
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                char* ext = strrchr(entry->d_name, '.');
                if (ext && strcasecmp(ext, ".szs") == 0) {
                    szsFound++;
                }
            }
        }
        closedir(dir);
        if (szsFound > 0) {
            printf("custom theme");
        } else {
            printf("stock theme");
        }
    } else {
        printf("stock theme");
    }
    setsysExit();
    consoleUpdate(NULL);
    // CPU
    tsInitialize();
    TsSession ts_session;
    tsOpenSession(&ts_session, TsDeviceCode_LocationExternal);
    float temp_c;
    tsSessionGetTemperature(&ts_session, &temp_c);
    tsExit();
    printf(CONSOLE_ESC(12;27H));
    printf("\e[38;5;33mCPU\e[38;5;255m: ARM 4 Cortex-A57 (4) @ %u MHz [%.1fC]",GetClock(PcvModule_CpuBus), temp_c);
    consoleUpdate(NULL);
    // GPU
    printf(CONSOLE_ESC(13;27H));
    printf("\e[38;5;33mGPU\e[38;5;255m: Nvidia GM20B @ %u MHz",GetClock(PcvModule_GPU));
    consoleUpdate(NULL);
    // Memory
    u64 totalRam = 0;
    u64 usedRam = 0;
    svcGetInfo(&totalRam, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
    svcGetInfo(&usedRam, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
    printf(CONSOLE_ESC(14;27H));
    printf("\e[38;5;33mMemory\e[38;5;255m: %ldMB / %ldMB @ %u MHz", (usedRam / 1024 / 1024), (totalRam / 1024 / 1024), GetClock(PcvModule_EMC));
    consoleUpdate(NULL);
    // Hardware
    setsysInitialize();
    SetSysProductModel model;
    setsysGetProductModel(&model);
    printf(CONSOLE_ESC(15;27H));
    printf("\e[38;5;33mHardware\e[38;5;255m: ");
    switch (model) {
        case SetSysProductModel_Iowa:printf("Iowa, Tegra X1+ (Mariko)"); break;
        case SetSysProductModel_Hoag:printf("Hoag, Tegra X1+ (Mariko)"); break;
        case SetSysProductModel_Calcio:printf("Calcio, Tegra X1+ (Mariko)"); break;
        case SetSysProductModel_Aula:printf("Aula, Tegra X1+ (Mariko)"); break;
        case SetSysProductModel_Nx:printf("Icosa, Tegra X1 (Erista)"); break;
        case SetSysProductModel_Copper:printf("Copper, Tegra X1 (Erista)"); break;
        default:printf("Unknown"); break;
    }
    setsysExit();
    // SD card
    FsFileSystem *sdFs = fsdevGetDeviceFileSystem("sdmc");
    s64 totalSpaceBytes, freeSpaceBytes;
    fsFsGetTotalSpace(sdFs, "/", &totalSpaceBytes);
    fsFsGetFreeSpace(sdFs, "/", &freeSpaceBytes);
    double totalSpaceGB = (double)totalSpaceBytes / (1024 * 1024 * 1024);
    double freeSpaceGB = (double)freeSpaceBytes / (1024 * 1024 * 1024);
    double leftSpaceGB = totalSpaceGB - freeSpaceGB;
    printf(CONSOLE_ESC(16;27H));
    printf("\e[38;5;33mDisk (sdmc:/)\e[38;5;255m: %.2fGB / %.2fGB ", leftSpaceGB, totalSpaceGB);
    if (leftSpaceGB / totalSpaceGB * 100 < 20) {
        printf("(\e[38;5;40m%.0f%%\e[38;5;255m)", leftSpaceGB / totalSpaceGB * 100);
    } else if (leftSpaceGB / totalSpaceGB * 100 >= 20 && leftSpaceGB / totalSpaceGB * 100 < 40) {
        printf("(\e[38;5;148m%.0f%%\e[38;5;255m)", leftSpaceGB / totalSpaceGB * 100);
    } else if (leftSpaceGB / totalSpaceGB * 100 >= 40 && leftSpaceGB / totalSpaceGB * 100 < 60) {
        printf("(\e[38;5;226m%.0f%%\e[38;5;255m)", leftSpaceGB / totalSpaceGB * 100);
    } else if (leftSpaceGB / totalSpaceGB * 100 >= 60 && leftSpaceGB / totalSpaceGB * 100 < 80) {
        printf("(\e[38;5;208m%.0f%%\e[38;5;255m)", leftSpaceGB / totalSpaceGB * 100);
    } else if (leftSpaceGB / totalSpaceGB * 100 >= 80) {
        printf("(\e[38;5;196m%.0f%%\e[38;5;255m)", leftSpaceGB / totalSpaceGB * 100);
    }
    consoleUpdate(NULL);
    // NAND USER
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
    printf(CONSOLE_ESC(17;27H));
    printf("\e[38;5;33mDisk (user:/)\e[38;5;255m: %.2fGB / %.2fGB ", leftSpaceGB2, totalSpaceGB2);
    if (leftSpaceGB2 / totalSpaceGB2 * 100 < 20) {
        printf("(\e[38;5;40m%.0f%%\e[38;5;255m)", leftSpaceGB2 / totalSpaceGB2 * 100);
    } else if (leftSpaceGB2 / totalSpaceGB2 * 100 >= 20 && leftSpaceGB2 / totalSpaceGB2 * 100 < 40) {
        printf("(\e[38;5;148m%.0f%%\e[38;5;255m)", leftSpaceGB2 / totalSpaceGB2 * 100);
    } else if (leftSpaceGB2 / totalSpaceGB2 * 100 >= 40 && leftSpaceGB2 / totalSpaceGB2 * 100 < 60) {
        printf("(\e[38;5;226m%.0f%%\e[38;5;255m)", leftSpaceGB2 / totalSpaceGB2 * 100);
    } else if (leftSpaceGB2 / totalSpaceGB2 * 100 >= 60 && leftSpaceGB2 / totalSpaceGB2 * 100 < 80) {
        printf("(\e[38;5;208m%.0f%%\e[38;5;255m)", leftSpaceGB2 / totalSpaceGB2 * 100);
    } else if (leftSpaceGB2 / totalSpaceGB2 * 100 >= 80) {
        printf("(\e[38;5;196m%.0f%%\e[38;5;255m)", leftSpaceGB2 / totalSpaceGB2 * 100);
    }
    consoleUpdate(NULL);
    // NAND SYSTEM
    fsInitialize();
    fsOpenBisFileSystem(&userFs, FsBisPartitionId_System, "");
    s64 totalSpaceBytes3, freeSpaceBytes3;
    fsFsGetTotalSpace(&userFs, "/", &totalSpaceBytes3);
    fsFsGetFreeSpace(&userFs, "/", &freeSpaceBytes3);
    double totalSpaceGB3 = (double)totalSpaceBytes3 / (1024 * 1024 * 1024);
    double freeSpaceGB3 = (double)freeSpaceBytes3 / (1024 * 1024 * 1024);
    double leftSpaceGB3 = totalSpaceGB3 - freeSpaceGB3;
    fsFsClose(&userFs);
    fsExit();
    printf(CONSOLE_ESC(18;27H));
    printf("\e[38;5;33mDisk (system:/)\e[38;5;255m: %.2fGB / %.2fGB ", leftSpaceGB3, totalSpaceGB3);
    if (leftSpaceGB3 / totalSpaceGB3 * 100 < 20) {
        printf("(\e[38;5;40m%.0f%%\e[38;5;255m)", leftSpaceGB3 / totalSpaceGB3 * 100);
    } else if (leftSpaceGB3 / totalSpaceGB3 * 100 >= 20 && leftSpaceGB3 / totalSpaceGB3 * 100 < 40) {
        printf("(\e[38;5;148m%.0f%%\e[38;5;255m)", leftSpaceGB3 / totalSpaceGB3 * 100);
    } else if (leftSpaceGB3 / totalSpaceGB3 * 100 >= 40 && leftSpaceGB3 / totalSpaceGB3 * 100 < 60) {
        printf("(\e[38;5;226m%.0f%%\e[38;5;255m)", leftSpaceGB3 / totalSpaceGB3 * 100);
    } else if (leftSpaceGB3 / totalSpaceGB3 * 100 >= 60 && leftSpaceGB3 / totalSpaceGB3 * 100 < 80) {
        printf("(\e[38;5;208m%.0f%%\e[38;5;255m)", leftSpaceGB3 / totalSpaceGB3 * 100);
    } else if (leftSpaceGB3 / totalSpaceGB3 * 100 >= 80) {
        printf("(\e[38;5;196m%.0f%%\e[38;5;255m)", leftSpaceGB3 / totalSpaceGB3 * 100);
    }
    consoleUpdate(NULL);
    // Battery
    psmInitialize();
    u32 batteryCharge;
    psmGetBatteryChargePercentage(&batteryCharge);
    printf(CONSOLE_ESC(19;27H));
    printf("\e[38;5;33mBattery\e[38;5;255m: ");
    if (batteryCharge >= 80) {
        printf("\e[38;5;40m%d%%\e[38;5;255m", batteryCharge);
    } else if (batteryCharge >= 60 && batteryCharge < 80) {
        printf("\e[38;5;148m%d%%\e[38;5;255m", batteryCharge);
    } else if (batteryCharge >= 40 && batteryCharge < 60) {
        printf("\e[38;5;226m%d%%\e[38;5;255m", batteryCharge);
    } else if (batteryCharge >= 20 && batteryCharge < 40) {
        printf("\e[38;5;208m%d%%\e[38;5;255m", batteryCharge);
    } else if (batteryCharge < 20) {
        printf("\e[38;5;196m%d%%\e[38;5;255m", batteryCharge);
    }
    PsmChargerType chargerType;
    psmGetChargerType(&chargerType);
    if (chargerType != PsmChargerType_Unconnected){
        printf(" [Charging]");
    } else {
        printf(" [Discharging]");
    }
    psmExit();
    consoleUpdate(NULL);
    // Local IP
    nifmInitialize(NifmServiceType_User);
    NifmInternetConnectionStatus status;
    rc = nifmGetInternetConnectionStatus(NULL, NULL, &status);
    printf(CONSOLE_ESC(20;27H));
    if (R_FAILED(rc)) {
        printf("\e[38;5;33mLocal IP\e[38;5;255m: Not connected!");
    } else if (status == NifmInternetConnectionStatus_Connected) {
        u32 local_ip = gethostid();
        struct in_addr addr;
        addr.s_addr = local_ip;
        char* ip_str = inet_ntoa(addr);
        printf("\e[38;5;33mLocal IP\e[38;5;255m: %s", ip_str);
    } else {
        printf("\e[38;5;33mLocal IP\e[38;5;255m: Not connected!");
    }
    nifmExit();
    consoleUpdate(NULL);
    // Locale
    setInitialize();
    u64 languageCode = 0;
    rc = setGetSystemLanguage(&languageCode);
    SetLanguage makeLanguage = 0;
    rc = setMakeLanguage(languageCode, &makeLanguage);
    SetRegion regionCode = 0;
    rc = setGetRegionCode(&regionCode);
    printf(CONSOLE_ESC(21;27H));
    printf("\e[38;5;33mLocale\e[38;5;255m: %s_%s\n", GetLanguageName(makeLanguage), getRegionName(regionCode));
    setExit();
    consoleUpdate(NULL);
    //Controllers
    printf(CONSOLE_ESC(22;27H));
    printf("\e[38;5;33mControllers\e[38;5;255m: ");
    for (int i = 0; i < 8; i++) {
        char name[16];
        snprintf(name, sizeof(name), "P%d", i+1);
        printController(hidGetNpadDeviceType(HidNpadIdType_No1 + i), name, HidNpadIdType_No1 + i);
    }
    printController(hidGetNpadDeviceType(HidNpadIdType_Other), "Other", HidNpadIdType_Other);
    printController(hidGetNpadDeviceType(HidNpadIdType_Handheld), "Handheld", HidNpadIdType_Handheld);
    if (anycontroller == false){
        printf(CONSOLE_ESC(22;40H));
        printf("None");
    }
    consoleUpdate(NULL);
    // Fancy color blocks
    printf("\n\n");
    printf(CONSOLE_ESC(26C));
    printf("\e[48;5;235m   \e[48;5;1m   \e[48;5;2m   \e[48;5;3m   \e[48;5;4m   \e[48;5;5m   \e[48;5;6m   \e[48;5;7m   \e[48;5;0m");
    printf("\n");
    printf(CONSOLE_ESC(26C));
    printf("\e[48;5;8m   \e[48;5;9m   \e[48;5;10m   \e[48;5;11m   \e[48;5;12m   \e[48;5;13m   \e[48;5;14m   \e[48;5;15m   \e[48;5;0m");
    while (appletMainLoop()) {
        bool plusPressed = false;
        for (int i = 0; i < 8; i++) {
            padUpdate(&pads[i]);
            u64 kDown = padGetButtonsDown(&pads[i]);
            if (kDown & HidNpadButton_Plus) {
                plusPressed = true;                
                break;
            }
            if (kDown & HidNpadButton_AnyRight)  {
                if (selected != 17) {
                    selected = selected + 1;
                    printf(CONSOLE_ESC(1;1H));
                    updateAscii();
                }
            }
            if (kDown & HidNpadButton_AnyLeft) {
                if (selected != 1) {
                    selected = selected - 1;
                    printf(CONSOLE_ESC(1;1H));
                    updateAscii();
                }
            }
            if (kDown & HidNpadButton_L) {
                if (userflag == false){
                    if (selectedUser != 0) {
                        selectedUser -= 1;
                        printf(CONSOLE_ESC(1;1H));
                        updateAscii();
                    }
                }
            }
            if (kDown & HidNpadButton_R) {
                if (userflag == false) {
                    if (selectedUser != totalUsers - 1) {
                        selectedUser += 1;
                        printf(CONSOLE_ESC(1;1H));
                        updateAscii();
                    }
                }
            }
        }
        if (plusPressed) {
            break;
        }
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
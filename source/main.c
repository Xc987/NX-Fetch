#include <stdio.h>
#include <string.h>
#include <switch.h>

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
    
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        consoleUpdate(NULL);
    }
    consoleExit(NULL);
    return 0;
}
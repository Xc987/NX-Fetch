#include <stdio.h>
#include <string.h>
#include <switch.h>

int main(int argc, char **argv) {
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    printf(R"EOF(
   ######## #######
  ##      # #########
 #        # ##########
#   ###   # ###########
#  #####  # ###########
#  #####  # ###########
#  #####  # ###########
#   ###   # ###########
#         # ###########
#         # ####   ####
#         # ###     ###
#         # ###     ###
#         # ###     ###
#         # ####   ####
#         # ###########
#         # ###########
#         # ###########
 #        # ########## 
  ##      # #########  
   ######## #######    
        )EOF");
    
    // Main loop
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
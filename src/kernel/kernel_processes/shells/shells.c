#include "shells.h"

void emergency_shell(void) {
	//dump_kprocesses();
    log("[KERNEL]", "FAILED TO JUMP TO USERMODE\n\n", error);
    log("::", "emexOS couldn't load \"" SYSTEMLOCATE2 "\"\n", _d);
    log("::", "you are now being dropped into the emergencyshell...\n", _d);
    print("\n", SHELL_FG_COLOR);

    //first clear screen
    //clear(0xff000000);

    print("At the top you should see a list like this\n", SHELL_FG_COLOR);
    print("Process List:\n", SHELL_FG_COLOR);
    print("PID   State       Name            Entry Point\n", SHELL_FG_COLOR);
    print("----  ----------  --------------  -----------\n", SHELL_FG_COLOR);
    print("3     READY       app.elf         0x400A0000\n", SHELL_FG_COLOR);
    print("2     READY       __rt            0x40001000\n", SHELL_FG_COLOR);
    print("1     READY       kernel          0x40000000\n\n", SHELL_FG_COLOR);

    print(" If your list looks the same as this, the jump likely failed.\n", SHELL_FG_COLOR);
    print(" In that case, try rebooting the system.\n\n", SHELL_FG_COLOR);

    print(" If your list looks different, there may be a hardware problem.\n", SHELL_FG_COLOR);
    print(" Please open an issue and include as much information as possible.\n\n", SHELL_FG_COLOR);

    print(" If rebooting multiple times does not fix the problem, also open an issue:\n", SHELL_FG_COLOR);
    print(" > https://github.com/emexos/emexOS/issues\n\n", SHELL_FG_COLOR);

    print(PROMPT, SHELL_FG_COLOR);

    //__builtin_unreachable();
}

void recovery_shell(void) {
	//dump_kprocesses();
	//
	log("::", "you are now being dropped into the recoveryshell...\n", _d);
	print("\n", SHELL_FG_COLOR);


	print(PROMPT, SHELL_FG_COLOR);
	//}

    //__builtin_unreachable();
}

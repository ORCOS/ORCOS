set verbose on
set backtrace limit 0

target extended-remote localhost:3333
load configurations/BeagleBoardxM/Prototype/tasks/shell_task/task.elf
load configurations/BeagleBoardxM/Prototype/output/kernel.elf
#symbol-file configurations/BeagleBoardxM/Prototype/output/kernel.elf
#detach
#set breakpoint auto-hw on
disconnect
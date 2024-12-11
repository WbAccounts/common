#include "proc_monitor.h"
#include "log/log_helper.h"

void deal_func(const struct ProcDealInfo& info) {
    LOG_INFO("event_type: {}", info.event_type);
    LOG_INFO("pid: {}", info.info.pid);
    LOG_INFO("tgid: {}", info.info.tgid);
    LOG_INFO("proc_name: {}", info.info.proc_name);
    LOG_INFO("proc_exe: {}", info.info.proc_exe);
    LOG_INFO("proc_cwd: {}", info.info.proc_cwd);
    LOG_INFO("proc_cmd: {}", info.info.proc_cmd);
};

int main () {
    // 程序需要root权限运行
    if (!LOG_HELPER->init_logger_cmd("proc.test")) {
        printf("init loger failed\n");
        return -1;
    }

    if (PROC_MONITOR_INSTANCE == NULL) {
        LOG_ERROR("proc monitor instance is null");
        return -1;
    }

    ProcSubInfo sub_info;
    sub_info.module = "test";
    sub_info.flow_map[proc_event::PROC_EVENT_EXEC] = {"monitor_test"};
    sub_info.flow_map[proc_event::PROC_EVENT_EXIT] = {"monitor_test"};
    sub_info.event_call[proc_event::PROC_EVENT_EXEC] = deal_func;
    sub_info.event_call[proc_event::PROC_EVENT_EXIT] = deal_func;

    if (PROC_MONITOR_INSTANCE->sub_proc(sub_info) != proc_error_ok) {
        LOG_ERROR("sub proc failed");
    }

    int a = 0;
    scanf("%d\n", &a);
    return 0;
}
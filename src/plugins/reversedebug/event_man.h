// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _EVENT_H
#define _EVENT_H
#include <inttypes.h>
#include <time.h>

#if !defined(__mips__)
#define __NR_Linux                    (0)
#else
#include <asm/unistd.h>
#endif

#define DUMP_REASON_syscall_exit      (-1000)
#define DUMP_REASON_signal            (__NR_Linux         + 1000)
#define DUMP_REASON_dbus              (DUMP_REASON_signal + 1000)
#define DUMP_REASON_x11               (DUMP_REASON_dbus   + 1000)
#define DUMP_REASON_ptrace            (DUMP_REASON_x11    + 1000)
#define DUMP_REASON_INVALID           (0xffff)

#define MAP_FILE_NAME                 "map.dump"
#define CONTEXT_FILE_NAME             "context.dump"
#define X11DBUS_FILE_NAME             "other.dump"
#define EXEC_FILE_NAME                "hardlink-"
#define LATEST_TRACE_NAME             "latest-trace"
#define EVENT_EXTRA_INFO_SIZE         (512)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagEventEntry {
    double              time;      // unit is millisecond
    double              duration;  // unit is millisecond
    int16_t             type;
    uint16_t            thread_num;
    uint16_t            tid; /* id of the thread event occurred */
    uint16_t            extra_size;
    long                offset; /* offset in trace file */
    long                syscall_result;
}EventEntry;

int create_timeline(const char *maps_file,
        const char *context_file, void **pp_timeline);
int destroy_timeline(void *timeline);
int get_event(void *timeline, int index, EventEntry *entry);
const EventEntry *get_event_pointer(void *timeline);
int get_event_extra_info(void *timeline, int index, char *buf, int buf_size);
int generate_coredump(void *timeline, int index, const char *corefile, int verbose);

// debug functions
const char *get_event_name(int type);
const char *syscall_name(long syscall_no);
const char *signal_name(int sig);
const char *errno_name(int err);
const char *sicode_name(int code, int sig);
const char *ptrace_event_name(int event);
#ifdef __cplusplus
}
#endif

#endif // #ifndef _EVENT_H

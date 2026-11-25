// qlFetch
// 
// Copyright (c) 2026, Atarwn Gard
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

#define HOST "/etc/hostname"

static void asciilogo(void) {
    FILE *f = fopen("/etc/logo", "r");
    if (!f) return;
    char buf[256];
    while (fgets(buf, sizeof buf, f))
        fputs(buf, stdout);
    fclose(f);
}

static void pretty_name(void) {
    FILE *f = fopen("/etc/os-release", "r");
    if (!f) return;
    char buf[256];
    while (fgets(buf, sizeof buf, f))
        if (!strncmp(buf, "PRETTY_NAME=", 12)) {
            buf[strcspn(buf, "\n")] = 0;
            char *p = buf + 12;
            p += strspn(p, "\"");
            p[strcspn(p, "\"")] = 0;
            printf("OS:      %s\n", p);
            break;
        }
    fclose(f);
}

static void kernel(void) {
    struct utsname u;
    if (!uname(&u))
        printf("Kernel:  %s %s\n", u.sysname, u.release);
}

static void uptime(void) {
    FILE *f = fopen("/proc/uptime", "r");
    if (!f) return;
    double up;
    if (fscanf(f, "%lf", &up) == 1) {
        long d = (long)up / 86400, h = ((long)up % 86400) / 3600, m = ((long)up % 3600) / 60;
        printf("Uptime:  %ldd %ldh %ldm\n", d, h, m);
    }
    fclose(f);
}

static void shell(void) {
    const char *s = getenv("SHELL");
    if (s && *s) {
        const char *p = strrchr(s, '/');
        printf("Shell:   %s\n", p ? p + 1 : s);
    }
}

static void packages(void) {
    /* считаем *.list в /var/db/pkgs — для QL-подобных систем.
       для deb/rpm поправь под себя */
    FILE *p = popen("ls /var/db/pkgs/*.list 2>/dev/null | wc -l", "r");
    if (!p) return;
    long n = 0;
    if (fscanf(p, "%ld", &n) == 1 && n > 0)
        printf("Packages: %ld\n", n);
    fclose(p);
}

static void host(void) {
    char h[256] = {0}, d[256] = {0};
    FILE *f = fopen(HOST, "r");
    if (!f) f = popen("hostname", "r");
    if (f && fgets(h, sizeof h, f)) h[strcspn(h, "\n")] = 0;
    if (f) fclose(f);
    if (gethostname(d, sizeof d) == 0)
        printf("Host:    %s (%s)\n", h[0] ? h : "?", d);
}

static void cpu(void) {
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) return;
    char buf[256], model[256] = "";
    int n = 0;
    while (fgets(buf, sizeof buf, f))
        if (!strncmp(buf, "model name", 10))
            sscanf(buf, "model name : %255[^\n]", model);
        else if (!strncmp(buf, "processor", 9))
            n++;
    if (model[0])
        printf("CPU:     %s (%d)\n", model, n);
    fclose(f);
}

static void memory(void) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return;
    char buf[256];
    long total = 0, avail = 0;
    while (fgets(buf, sizeof buf, f)) {
        if (!strncmp(buf, "MemTotal:", 9)) sscanf(buf, "MemTotal: %ld", &total);
        if (!strncmp(buf, "MemAvailable:", 13)) sscanf(buf, "MemAvailable: %ld", &avail);
    }
    if (total)
        printf("Memory:  %ldM / %ldM\n",
               (total - avail) / 1024, total / 1024);
    fclose(f);
}

int main(void) {
    asciilogo();
    pretty_name();
    kernel();
    uptime();
    shell();
    packages();
    host();
    cpu();
    memory();
    return 0;
}

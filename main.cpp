#include <cstdio>
#include <X11/Xlib.h>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cstring>
#include <dirent.h>
#include <regex.h>
#include <list>
#include <thread>
#include <iostream>
#include "boost/thread.hpp"
#include <boost/ptr_container/ptr_list.hpp>

struct event_device {
    char *device;
    int fd;
};

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

static const char *const evval[3] = {
        "RELEASED",
        "PRESSED ",
        "REPEATED"
};

void listenDevice(event_device evdev) {
    struct input_event ev[64];
    int numevents;
    int result = 0;
    int size = sizeof(struct input_event);
    int rd;
    char name[256];
    fd_set fds{};
    int maxfd;

    if (evdev.fd == -1) {
        die("Failed to open event device");
    }

    memset(name, 0, sizeof(name));
    ioctl(evdev.fd, EVIOCGNAME(sizeof(name)), name);
    if (name)
    printf("Reading From : %s (%s)\n", evdev.device, name);

    while (true) {
        FD_ZERO(&fds);
        maxfd = -1;

        FD_SET(evdev.fd, &fds);
        if (maxfd < evdev.fd) maxfd = evdev.fd;

        result = select(maxfd + 1, &fds, nullptr, nullptr, nullptr);
        if (result == -1) {
            die("");
        }


        if (!FD_ISSET(evdev.fd, &fds)) {
            die("");
        }

        if ((rd = read(evdev.fd, ev, size * 64)) < size) {
            die("");
        }

        numevents = rd / size;

        for (int j = 0; j < numevents; ++j) {
//            if (ev[j].value > 0 && ev[j].type == 4) {
//                printf("%s: Type[%d] Code[%d] Value[%d]\n", evdev.device, ev[j].type, ev[j].code, ev[j].value);
//            }
            if (ev[j].type == EV_KEY && ev[j].value >= 0 && ev[j].value <= 2) {
//                if (ev[j].value == 0) {
                printf("%s 0x%04x (%d)\n", evval[ev[j].value], (int) ev[j].code, (int) ev[j].code);
//                }
            }

        }
    }
}

int main(int argc, char *argv[]) {
    std::vector<std::string> devices;
    boost::thread_group tgroup;

    struct event_device evdev{};

    DIR *dirp;
    static const char *dirName = "/dev/input";
    char fullPath[1024];
    struct dirent *dp;
    regex_t kbd{};

    if (regcomp(&kbd, "event", 0) != 0) {
        die("regcomp for kbd failed");
    }

    if ((dirp = opendir(dirName)) == nullptr) {
        die("couldn't open '/dev/input/by-id'");
    }

    do {
        if ((dp = readdir(dirp)) != nullptr) {
            if (regexec(&kbd, dp->d_name, 0, nullptr, 0) == 0) {
                sprintf(fullPath, "%s/%s", dirName, dp->d_name);
                devices.emplace_back(std::string(fullPath, sizeof(fullPath)));
            }
        }
    } while (dp != nullptr);

    for (const auto &device : devices) {
        evdev.device = const_cast<char *>(device.c_str());
        evdev.fd = open(evdev.device, O_RDONLY | O_NONBLOCK);
        tgroup.create_thread(boost::bind(&listenDevice, evdev));
    }
    tgroup.join_all();

//    printf("Exiting.\n");
//
//    ioctl(evdev.fd, EVIOCGRAB, 0);
//    close(evdev.fd);
//
//    return 0;
}
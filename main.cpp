#include <cstdio>
#include <X11/Xlib.h>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cstring>
#include <list>
#include <iostream>
#include "boost/thread.hpp"
#include <boost/ptr_container/ptr_list.hpp>
#include <fstream>
#include <csignal>
#include <unordered_map>
#include "UpdateListener.h"

const char *inputPath = "/dev/input/";

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

std::vector<char *> extract_keyboard_events() {
    FILE *fp = NULL;
    char buffer[1024];
    std::vector<char *> events;
    char *eventname = NULL;
    fp = fopen("/proc/bus/input/devices", "r");
    if (!fp) {
        int err = errno;
        fprintf(stderr, "Unable to open file. %s\n", strerror(err));
        die("Unable to open file");
    }
    memset(buffer, 0, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), fp)) {
        char *ptr = NULL;
        if ((ptr = strstr(buffer, "Handlers="))) {
            ptr += strlen("Handlers=");
            ptr = strstr(ptr, "event");
            if (ptr) {
                char *ptr2 = strchr(ptr, ' ');
                if (ptr2) *ptr2 = '\0';
                eventname = strdup(ptr);
                if (!eventname) {
                    fprintf(stderr, "Out of memory.\n");
                    break;
                }
            }
        }
        if (strstr(buffer, "EV=120013")) {
            events.emplace_back(eventname);
        }
    }
    fclose(fp);
    return events;
}

bool is_keyboard_event(char *eventName) {
    for (auto *event : extract_keyboard_events()) {
        if (strcmp(event, eventName) == 0) {
            return true;
        }
    }
    return false;
}

void listenDevice(char *event) {
    struct input_event ev[64];
    int numEvents;
    int result = 0;
    int size = sizeof(struct input_event);
    int rd;
    char name[256];
    fd_set fds{};
    int maxfd;
    struct event_device evdev{};
//    const char* inputPath = "/dev/input/";

//    char device[1024];
//    memset(device, 0, sizeof(device));
//    sprintf(device, "%s%s", inputPath, event);

    char *device = (char *) malloc(1 + strlen(inputPath) + strlen(reinterpret_cast<const char *>(event)));
    strcpy(device, inputPath);
    strcat(device, reinterpret_cast<const char *>(event));

    evdev.device = device;
    evdev.fd = open(evdev.device, O_RDONLY | O_NONBLOCK);

    if (evdev.fd == -1) {
        try {
            boost::this_thread::interruption_point();
        } catch (boost::thread_interrupted &) {
            std::cout << "interrupted0" << std::endl;
        }
    }

    memset(name, 0, sizeof(name));
    ioctl(evdev.fd, EVIOCGNAME(sizeof(name)), name);

    printf("Reading From : %s (%s)\n", evdev.device, name);

    while (true) {
        FD_ZERO(&fds);
        maxfd = -1;

        FD_SET(evdev.fd, &fds);
        if (maxfd < evdev.fd) maxfd = evdev.fd;
        result = select(maxfd + 1, &fds, nullptr, nullptr, nullptr);
        if (result == -1) {
            try {
                boost::this_thread::interruption_point();
            } catch (boost::thread_interrupted &) {
                std::cout << "interrupted1" << std::endl;
            }
        }

        if (!FD_ISSET(evdev.fd, &fds)) {
            try {
                boost::this_thread::interruption_point();
            } catch (boost::thread_interrupted &) {
                std::cout << "interrupted2" << std::endl;
            }
        }
        if ((rd = read(evdev.fd, ev, size * 64)) < size) {
            try {
                boost::this_thread::interruption_point();
            } catch (boost::thread_interrupted &) {
                std::cout << "interrupted3" << std::endl;
            }
        }

        numEvents = rd / size;
        for (int j = 0; j < numEvents; ++j) {
//            if (ev[j].value > 0 && ev[j].type == 4) {
//                printf("%s: Type[%d] Code[%d] Value[%d]\n", evdev.device, ev[j].type, ev[j].code, ev[j].value);
//            }
            if (ev[j].type == EV_KEY && ev[j].value >= 0 && ev[j].value <= 2) {
//                if (ev[j].value == 0) {
                printf("From : %s (%s)\n", evdev.device, name);
                printf("%s 0x%04x (%d)\n", evval[ev[j].value], (int) ev[j].code, (int) ev[j].code);
//                }
            }

        }
    }
}

struct cmp_str {
    bool operator()(char const *a, char const *b) {
        return std::strcmp(a, b) < 0;
    }
};

void create_watcher() {
    auto *fileWatcher = new efsw::FileWatcher();
    auto *listener = new UpdateListener();
    efsw::WatchID watchID = fileWatcher->addWatch(inputPath, listener, false);
    while (true) {
        fileWatcher->watch();
    }
}

int main(int argc, char *argv[]) {
    std::map<char *, boost::thread *, cmp_str> eventThreadMap;
//    boost::thread_group threadGroup;

//    if ((getuid()) != 0) {
//        die("You are not root!");
//    }

    for (auto *event : extract_keyboard_events()) {
//        threadGroup.create_thread(boost::bind(&listenDevice, event));
        eventThreadMap[event] = new boost::thread(boost::bind(&listenDevice, event));
    }

//    eventThreadMap.find((char *const &) "event2")->second.interrupt();

    for (auto &eventThread : eventThreadMap) {
        eventThread.second->interrupt();
        eventThread.second->detach();
//        threadGroup.add_thread(eventThread.second);
    }
    while (1) {}


//    threadGroup.create_thread(boost::bind(&create_watcher));

//    threadGroup.interrupt_all();
//    threadGroup.join_all();

//    for (auto *event : extract_keyboard_events()) {
//        threadGroup.create_thread(boost::bind(&listenDevice, event));
//    }
//    threadGroup.join_all();

//    ioctl(evdev.fd, EVIOCGRAB, 0);
//    close(evdev.fd);
//
//    return 0;
}
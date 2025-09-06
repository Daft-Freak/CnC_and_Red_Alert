#pragma once

// hack around some conflicting header gauards
#undef QUEUE_H
#undef LIST_H

#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#define gethostname(name, size) strncpy(name, "esp32", size)

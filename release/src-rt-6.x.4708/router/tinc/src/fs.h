#ifndef TINC_FS_H
#define TINC_FS_H

#include "system.h"

static const unsigned int DIR_CACHE       = 1 << 0;
static const unsigned int DIR_CONFBASE    = 1 << 1;
static const unsigned int DIR_CONFDIR     = 1 << 2;
static const unsigned int DIR_HOSTS       = 1 << 3;
static const unsigned int DIR_INVITATIONS = 1 << 4;

// Create one or multiple directories inside tincd configuration directory
extern bool makedirs(unsigned int dirs);

// Open file. If it does not exist, create a new file with the specified access mode.
extern FILE *fopenmask(const char *filename, const char *mode, mode_t perms) ATTR_DEALLOCATOR(fclose);

// Get absolute path to a possibly nonexistent file or directory
extern char *absolute_path(const char *path) ATTR_MALLOC;

#endif // TINC_FS_H

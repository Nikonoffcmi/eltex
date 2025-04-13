#include "manual.h"

#define DATA_FILE "contacts.dat"

void write_string(int fd, const char *str);
void read_string(int fd, char **str);

void save_contact(int fd, Contact *contact);
void load_contact(int fd, Manual *manual);

void save_manual(Manual *manual);
void load_manual(Manual *manual);
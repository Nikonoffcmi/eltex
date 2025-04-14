#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "save.h"

void write_string(int fd, const char *str) {
    int len = str ? strlen(str) + 1 : 0;
    write(fd, &len, sizeof(int));
    if(len > 0) write(fd, str, len);
}

void read_string(int fd, char **str) {
    int len;
    read(fd, &len, sizeof(int));
    if(len > 0) {
        *str = malloc(len);
        read(fd, *str, len);
    } else {
        *str = NULL;
    }
}

void save_contact(int fd, Contact *contact) {
    write(fd, &contact->ID, sizeof(int));
    
    write_string(fd, contact->person.name);
    write_string(fd, contact->person.last_name);
    write_string(fd, contact->person.patronymic);
    write_string(fd, contact->person.workplace);
    write_string(fd, contact->person.job);

    write(fd, &contact->email_counter, sizeof(int));
    for(int i = 0; i < contact->email_counter; i++)
        write_string(fd, contact->emails[i].email);

    write(fd, &contact->phone_numbers_counter, sizeof(int));
    for(int i = 0; i < contact->phone_numbers_counter; i++)
        write_string(fd, contact->phone_numbers[i].phone_number);

    write(fd, &contact->socials_counter, sizeof(int));
    for(int i = 0; i < contact->socials_counter; i++)
        write_string(fd, contact->socials[i].social);
}

void load_contact(int fd, Manual *manual) {
    Contact *contact = malloc(sizeof(Contact));
    
    read(fd, &contact->ID, sizeof(int));
    manual->id_counter = contact->ID >= manual->id_counter ? contact->ID + 1 : manual->id_counter;

    read_string(fd, &contact->person.name);
    read_string(fd, &contact->person.last_name);
    read_string(fd, &contact->person.patronymic);
    read_string(fd, &contact->person.workplace);
    read_string(fd, &contact->person.job);

    Init_contact(contact);

    read(fd, &contact->email_counter, sizeof(int));
    for(int i = 0; i < contact->email_counter; i++) {
        read_string(fd, &contact->emails[i].email);
    }

    read(fd, &contact->phone_numbers_counter, sizeof(int));
    for(int i = 0; i < contact->phone_numbers_counter; i++) {
        read_string(fd, &contact->phone_numbers[i].phone_number);
    }

    read(fd, &contact->socials_counter, sizeof(int));
    for(int i = 0; i < contact->socials_counter; i++) {
        read_string(fd, &contact->socials[i].social);
    }

    Reinsert_contact(manual, contact);
}

void save_manual(Manual *manual) {
    int fd = open(DATA_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        return;
    } 

    int count = 0;
    Contact *current = manual->head;
    while(current) { count++; current = current->next; }
    
    if (write(fd, &count, sizeof(int)) == -1) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    current = manual->head;
    while(current) {
        save_contact(fd, current);
        current = current->next;
    }
    close(fd);
}

void load_manual(Manual *manual) {
    int fd = open(DATA_FILE, O_RDONLY);
    if(fd == -1){
        perror("open");
        return;
    } 

    int count;
    if (read(fd, &count, sizeof(int)) == -1) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < count; i++) {
        load_contact(fd, manual);
    }
    close(fd);
}